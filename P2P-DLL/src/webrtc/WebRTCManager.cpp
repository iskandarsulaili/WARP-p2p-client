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
    bool initialized = false;
    int max_peers = 50;

    // Signaling callbacks
    OnOfferCallback on_offer;
    OnAnswerCallback on_answer;
    OnIceCandidateCallback on_ice_candidate;
};

WebRTCManager::WebRTCManager() : impl_(std::make_unique<Impl>()) {
    LOG_DEBUG("WebRTCManager created");
}

WebRTCManager::~WebRTCManager() {
    Shutdown();
}

bool WebRTCManager::Initialize(const std::vector<std::string>& stun, const std::vector<std::string>& turn, int max) {
    impl_->stun_servers = stun;
    impl_->turn_servers = turn;
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
    peer->Initialize(impl_->stun_servers, impl_->turn_servers);

    // Set up ICE candidate callback
    peer->SetOnIceCandidateCallback([this, peer_id](const std::string& candidate) {
        if (impl_->on_ice_candidate) {
            impl_->on_ice_candidate(peer_id, candidate);
        }
    });

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

bool WebRTCManager::BroadcastData(const uint8_t* data, size_t length) {
    if (!data || length == 0) {
        LOG_ERROR("Invalid data for broadcast");
        return false;
    }

    std::lock_guard<std::mutex> lock(impl_->mutex);

    if (impl_->peers.empty()) {
        LOG_WARN("No peers to broadcast to");
        return false;
    }

    int success_count = 0;
    int total_peers = 0;

    for (const auto& peer : impl_->peers) {
        if (peer && peer->IsConnected()) {
            total_peers++;
            if (peer->SendData(data, length)) {
                success_count++;
            } else {
                LOG_WARN("Failed to send data to peer: " + peer->GetPeerId());
            }
        }
    }

    if (total_peers == 0) {
        LOG_WARN("No connected peers to broadcast to");
        return false;
    }

    LOG_DEBUG("Broadcast data to " + std::to_string(success_count) + "/" +
              std::to_string(total_peers) + " peers");

    return success_count > 0;
}

bool WebRTCManager::SendDataToPeer(const std::string& peer_id, const uint8_t* data, size_t length) {
    if (!data || length == 0) {
        LOG_ERROR("Invalid data for send");
        return false;
    }

    auto peer = GetPeerConnection(peer_id);
    if (!peer) {
        LOG_ERROR("Peer not found: " + peer_id);
        return false;
    }

    if (!peer->IsConnected()) {
        LOG_ERROR("Peer not connected: " + peer_id);
        return false;
    }

    bool success = peer->SendData(data, length);
    if (!success) {
        LOG_ERROR("Failed to send data to peer: " + peer_id);
    }

    return success;
}

bool WebRTCManager::CreateOffer(const std::string& peer_id) {
    LOG_INFO("Creating WebRTC offer for peer: " + peer_id);

    auto peer_conn = GetPeerConnection(peer_id);
    if (!peer_conn) {
        // Create new peer connection
        peer_conn = CreatePeerConnection(peer_id);
        if (!peer_conn) {
            LOG_ERROR("Failed to create peer connection for: " + peer_id);
            return false;
        }
    }

    // Create the offer
    std::string sdp;
    if (!peer_conn->CreateOffer(sdp)) {
        LOG_ERROR("Failed to create offer for peer: " + peer_id);
        return false;
    }

    LOG_INFO("Created offer for peer: " + peer_id + ", SDP length: " + std::to_string(sdp.length()));

    // Send the offer through the callback
    if (impl_->on_offer) {
        impl_->on_offer(peer_id, sdp);
    }

    return true;
}

bool WebRTCManager::HandleOffer(const std::string& peer_id, const std::string& sdp) {
    LOG_INFO("Handling WebRTC offer from peer: " + peer_id + ", SDP length: " + std::to_string(sdp.length()));

    auto peer_conn = GetPeerConnection(peer_id);
    if (!peer_conn) {
        // Create new peer connection for incoming offer
        peer_conn = CreatePeerConnection(peer_id);
        if (!peer_conn) {
            LOG_ERROR("Failed to create peer connection for: " + peer_id);
            return false;
        }
    }

    // Set remote description (offer)
    if (!peer_conn->SetRemoteDescription(sdp)) {
        LOG_ERROR("Failed to set remote description (offer) for peer: " + peer_id);
        return false;
    }

    // Create answer
    std::string answer_sdp;
    if (!peer_conn->CreateAnswer(answer_sdp)) {
        LOG_ERROR("Failed to create answer for peer: " + peer_id);
        return false;
    }

    LOG_INFO("Created answer for peer: " + peer_id + ", SDP length: " + std::to_string(answer_sdp.length()));

    // Send the answer through the callback
    if (impl_->on_answer) {
        impl_->on_answer(peer_id, answer_sdp);
    }

    return true;
}

bool WebRTCManager::HandleAnswer(const std::string& peer_id, const std::string& sdp) {
    LOG_INFO("Handling WebRTC answer from peer: " + peer_id + ", SDP length: " + std::to_string(sdp.length()));

    auto peer_conn = GetPeerConnection(peer_id);
    if (!peer_conn) {
        LOG_ERROR("No peer connection found for: " + peer_id);
        return false;
    }

    // Set remote description (answer)
    if (!peer_conn->SetRemoteDescription(sdp)) {
        LOG_ERROR("Failed to set remote description (answer) for peer: " + peer_id);
        return false;
    }

    LOG_INFO("Successfully processed answer from peer: " + peer_id);
    return true;
}

bool WebRTCManager::AddIceCandidate(const std::string& peer_id, const std::string& candidate,
                                   const std::string& sdp_mid, int sdp_mline_index) {
    LOG_DEBUG("Adding ICE candidate for peer: " + peer_id + ", candidate: " + candidate);

    auto peer_conn = GetPeerConnection(peer_id);
    if (!peer_conn) {
        LOG_WARN("No peer connection found for ICE candidate from: " + peer_id);
        return false;
    }

    // Add ICE candidate to peer connection
    if (!peer_conn->AddIceCandidate(candidate)) {
        LOG_ERROR("Failed to add ICE candidate for peer: " + peer_id);
        return false;
    }

    LOG_DEBUG("Successfully added ICE candidate for peer: " + peer_id);
    return true;
}

void WebRTCManager::CloseConnection(const std::string& peer_id) {
    LOG_INFO("Closing connection to peer: " + peer_id);
    RemovePeerConnection(peer_id);
}

void WebRTCManager::CloseAllConnections() {
    LOG_INFO("Closing all peer connections");

    auto peers = GetConnectedPeers();
    for (const auto& peer_id : peers) {
        RemovePeerConnection(peer_id);
    }
}

void WebRTCManager::SetOnOfferCallback(OnOfferCallback callback) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->on_offer = callback;
}

void WebRTCManager::SetOnAnswerCallback(OnAnswerCallback callback) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->on_answer = callback;
}

void WebRTCManager::SetOnIceCandidateCallback(OnIceCandidateCallback callback) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->on_ice_candidate = callback;
}

} // namespace P2P
