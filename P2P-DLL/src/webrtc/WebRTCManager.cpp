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

} // namespace P2P
