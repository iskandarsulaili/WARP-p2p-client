#include "../../include/WebRTCPeerConnection.h"
#include "../../include/Logger.h"

namespace P2P {

struct WebRTCPeerConnection::Impl {
    std::string peer_id;
    bool connected = false;
    bool initialized = false;
};

WebRTCPeerConnection::WebRTCPeerConnection(const std::string& peer_id) 
    : impl_(std::make_unique<Impl>()) {
    impl_->peer_id = peer_id;
    LOG_DEBUG("WebRTCPeerConnection created for: " + peer_id);
}

WebRTCPeerConnection::~WebRTCPeerConnection() {
    Close();
}

bool WebRTCPeerConnection::Initialize(const std::vector<std::string>& stun, const std::vector<std::string>& turn) {
    impl_->initialized = true;
    LOG_INFO("WebRTCPeerConnection initialized for: " + impl_->peer_id);
    return true;
}

void WebRTCPeerConnection::Close() {
    impl_->connected = false;
    impl_->initialized = false;
}

bool WebRTCPeerConnection::CreateOffer(std::string& sdp_out) {
    sdp_out = "v=0\r\no=- 0 0 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\n";
    return true;
}

bool WebRTCPeerConnection::CreateAnswer(std::string& sdp_out) {
    sdp_out = "v=0\r\no=- 0 0 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\n";
    return true;
}

bool WebRTCPeerConnection::SetRemoteDescription(const std::string& sdp) {
    LOG_DEBUG("SetRemoteDescription for: " + impl_->peer_id);
    return true;
}

bool WebRTCPeerConnection::AddIceCandidate(const std::string& candidate) {
    LOG_DEBUG("AddIceCandidate for: " + impl_->peer_id);
    return true;
}

bool WebRTCPeerConnection::SendData(const uint8_t* data, size_t size) {
    if (!impl_->connected) return false;
    LOG_DEBUG("SendData: " + std::to_string(size) + " bytes");
    return true;
}

bool WebRTCPeerConnection::IsConnected() const {
    return impl_->connected;
}

std::string WebRTCPeerConnection::GetPeerId() const {
    return impl_->peer_id;
}

void WebRTCPeerConnection::SetOnDataCallback(OnDataCallback callback) {
    // Store callback
}

void WebRTCPeerConnection::SetOnStateChangeCallback(OnStateChangeCallback callback) {
    // Store callback
}

void WebRTCPeerConnection::SetOnIceCandidateCallback(OnIceCandidateCallback callback) {
    // Store callback
}

} // namespace P2P
