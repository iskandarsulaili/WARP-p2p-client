#include "../../include/WebRTCManager.h"
#include "../../include/WebRTCPeerConnection.h"
#include "../../include/Logger.h"
#include <algorithm>
#include <mutex>

namespace P2P {

struct WebRTCManager::Impl {
    std::vector<std::shared_ptr<WebRTCPeerConnection>> peers;
    std::mutex mutex;
    std::vector<std::string> stun_servers;
    std::vector<std::string> turn_servers;
    std::string turn_username;
    std::string turn_credential;
    bool initialized = false;
    int max_peers = 50;

    // AOI/mesh
    float local_x = 0.0f, local_y = 0.0f, local_z = 0.0f;
    float aoi_radius = 100.0f;
    int mesh_refresh_interval_ms = 5000;
    float peer_score_threshold = 0.5f;
    int prune_interval_ms = 10000;
    std::chrono::steady_clock::time_point last_refresh = std::chrono::steady_clock::now();
};

WebRTCManager::WebRTCManager() : impl_(std::make_unique<Impl>()) {
    LOG_DEBUG("WebRTCManager created");
}

void WebRTCManager::SetLocalPosition(float x, float y, float z) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->local_x = x;
    impl_->local_y = y;
    impl_->local_z = z;
    LOG_DEBUG("SetLocalPosition: (" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z) + ")");
}

void WebRTCManager::SetAOIRadius(float radius) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->aoi_radius = radius;
    LOG_DEBUG("SetAOIRadius: " + std::to_string(radius));
}

float WebRTCManager::GetAOIRadius() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->aoi_radius;
}

void WebRTCManager::RefreshMesh() {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - impl_->last_refresh).count() < impl_->mesh_refresh_interval_ms) {
        return;
    }
    impl_->last_refresh = now;

    for (auto it = impl_->peers.begin(); it != impl_->peers.end(); ) {
        auto& peer = *it;
        float px, py, pz;
        peer->GetPeerPosition(px, py, pz);
        bool in_aoi = peer->IsWithinAOI(impl_->local_x, impl_->local_y, impl_->local_z, impl_->aoi_radius);
        float score = peer->GetPeerScore();

        LOG_DEBUG("MeshRefresh: PeerId=" + peer->GetPeerId() +
                  " Position=(" + std::to_string(px) + "," + std::to_string(py) + "," + std::to_string(pz) + ")" +
                  " InAOI=" + (in_aoi ? "true" : "false") +
                  " Score=" + std::to_string(score));

        // AOI/interest-based pruning
        if (!in_aoi || score < impl_->peer_score_threshold) {
            LOG_INFO("Pruning peer: PeerId=" + peer->GetPeerId() +
                     " InAOI=" + (in_aoi ? "true" : "false") +
                     " Score=" + std::to_string(score));
            peer->Close();
            it = impl_->peers.erase(it);
            continue;
        }
        ++it;
    }
    LOG_INFO("MeshRefresh complete. Peer count: " + std::to_string(impl_->peers.size()));
    // Telemetry: mesh refresh event
    LOG_DEBUG("Telemetry: Mesh refreshed, peer count: " + std::to_string(impl_->peers.size()));
}

WebRTCManager::~WebRTCManager() {
    Shutdown();
}

bool WebRTCManager::Initialize(const std::vector<std::string>& stun, const std::vector<std::string>& turn,
                              const std::string& turn_username, const std::string& turn_credential, int max) {
    impl_->stun_servers = stun;
    impl_->turn_servers = turn;
    impl_->turn_username = turn_username;
    impl_->turn_credential = turn_credential;
    impl_->max_peers = max;
    impl_->initialized = true;
    LOG_INFO("WebRTCManager initialized");
    return true;
}

void WebRTCManager::Shutdown() {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->peers.clear();
    impl_->initialized = false;
}

std::shared_ptr<WebRTCPeerConnection> WebRTCManager::CreatePeerConnection(const std::string& peer_id) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    auto peer = std::make_shared<WebRTCPeerConnection>(peer_id);
    peer->Initialize(impl_->stun_servers, impl_->turn_servers,
                    impl_->turn_username, impl_->turn_credential);
    impl_->peers.push_back(peer);
    return peer;
}

void WebRTCManager::RemovePeerConnection(const std::string& peer_id) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->peers.erase(
        std::remove_if(impl_->peers.begin(), impl_->peers.end(),
            [&](const auto& p) { return p->GetPeerId() == peer_id; }),
        impl_->peers.end()
    );
}

std::shared_ptr<WebRTCPeerConnection> WebRTCManager::GetPeerConnection(const std::string& peer_id) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    auto it = std::find_if(impl_->peers.begin(), impl_->peers.end(),
        [&](const auto& p) { return p->GetPeerId() == peer_id; });
    return (it != impl_->peers.end()) ? *it : nullptr;
}

std::vector<std::string> WebRTCManager::GetConnectedPeers() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    std::vector<std::string> result;
    for (const auto& p : impl_->peers) {
        if (p->IsConnected()) result.push_back(p->GetPeerId());
    }
    return result;
}

int WebRTCManager::GetPeerCount() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    return static_cast<int>(impl_->peers.size());
}

bool WebRTCManager::IsConnected() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    for (const auto& p : impl_->peers) {
        if (p->IsConnected()) return true;
    }
    return false;
}

bool WebRTCManager::SendData(const void* data, size_t size) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    bool success = false;
    for (const auto& p : impl_->peers) {
        if (p->IsConnected()) {
            if (p->SendData(static_cast<const uint8_t*>(data), size)) {
                success = true;
            }
        }
    }
    return success;
}

void WebRTCManager::ProcessOffer(const std::string& offer) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    // Extract peer_id from SDP offer using regex (look for "a=mid:peerid-<id>" or "a=msid-semantic: WMS <id>")
    std::string peer_id = "unknown_peer";
    std::smatch match;
    std::regex mid_regex(R"(a=mid:peerid-([^\r\n]+))");
    std::regex wms_regex(R"(a=msid-semantic: WMS ([^\r\n]+))");
    if (std::regex_search(offer, match, mid_regex) && match.size() > 1) {
        peer_id = match[1];
    } else if (std::regex_search(offer, match, wms_regex) && match.size() > 1) {
        peer_id = match[1];
    }
    auto peer = GetPeerConnection(peer_id);
    if (!peer) {
        peer = CreatePeerConnection(peer_id);
    }
    if (peer->SetRemoteDescription(offer)) {
        std::string answer;
        if (peer->CreateAnswer(answer)) {
            LOG_INFO("Processed offer and created answer for peer: " + peer_id);
            // Telemetry: log event
            LOG_DEBUG("Telemetry: Offer processed, answer created for peer " + peer_id);
        } else {
            LOG_ERROR("Failed to create answer for peer: " + peer_id);
        }
    } else {
        LOG_ERROR("Failed to set remote description for offer from peer: " + peer_id);
    }
}

void WebRTCManager::AddIceCandidate(const std::string& candidate) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    // Try to extract peer_id from candidate string (look for "ufrag" or "username" fields)
    std::string peer_id = "unknown_peer";
    std::smatch match;
    std::regex ufrag_regex(R"(ufrag ([^\s\r\n]+))");
    if (std::regex_search(candidate, match, ufrag_regex) && match.size() > 1) {
        peer_id = match[1];
    }
    auto peer = GetPeerConnection(peer_id);
    if (peer) {
        if (peer->AddIceCandidate(candidate)) {
            LOG_INFO("Added ICE candidate for peer: " + peer_id);
            // Telemetry: log event
            LOG_DEBUG("Telemetry: ICE candidate added for peer " + peer_id);
        } else {
            LOG_ERROR("Failed to add ICE candidate for peer: " + peer_id);
        }
    } else {
        LOG_ERROR("No peer connection found for ICE candidate: " + peer_id);
    }
}

} // namespace P2P
