// WebRTCPeerConnection.cpp - Production implementation using libdatachannel and msquic
#include "../../include/WebRTCPeerConnection.h"
#include "../../include/Logger.h"
#include <rtc/rtc.hpp>
// #include <msquic.h> // msquic integration is disabled for clean build
#include <mutex>
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <atomic>

namespace P2P {


struct WebRTCPeerConnection::Impl {
    std::string peer_id;
    std::shared_ptr<rtc::PeerConnection> pc;
    std::shared_ptr<rtc::DataChannel> dc;
    // msquic/QUIC transport is disabled for this build
    bool connected = false;
    bool initialized = false;

    // AOI/mesh
    float pos_x = 0.0f, pos_y = 0.0f, pos_z = 0.0f;
    float score = 1.0f;

    OnDataCallback on_data;
    OnStateChangeCallback on_state_change;
    OnIceCandidateCallback on_ice_candidate;
    OnPacketCallback on_packet;

    // Anti-cheat/reputation
    float anomaly_score = 0.0f;
    int suspicious_packet_count = 0;
    int total_packet_count = 0;
    std::chrono::steady_clock::time_point last_anomaly_check = std::chrono::steady_clock::now();

    std::mutex mutex;
    std::string local_sdp;
    bool gathering_complete = false;
};

WebRTCPeerConnection::WebRTCPeerConnection(const std::string& peer_id)
    : impl_(std::make_unique<Impl>()) {
    impl_->peer_id = peer_id;
    LOG_DEBUG("WebRTCPeerConnection created for: " + peer_id);
}

WebRTCPeerConnection::~WebRTCPeerConnection() {
    Close();
}

bool WebRTCPeerConnection::Initialize(const std::vector<std::string>& stun, const std::vector<std::string>& turn,
                                      const std::string& turn_username, const std::string& turn_credential) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    rtc::Configuration config;
    // libdatachannel >=0.17: iceServers is a vector of urls directly
    for (const auto& s : stun) {
        config.iceServers.push_back(s);
    }
    for (const auto& t : turn) {
        // Parse TURN URL of the form "turn:host:port"
        std::string url = t;
        std::string host;
        uint16_t port = 3478;
        if (url.rfind("turn:", 0) == 0) url = url.substr(5);
        auto colon = url.find(':');
        if (colon != std::string::npos) {
            host = url.substr(0, colon);
            port = static_cast<uint16_t>(std::stoi(url.substr(colon + 1)));
        } else {
            host = url;
        }
        config.iceServers.emplace_back(host, port, turn_username, turn_credential);
    }

    try {
        impl_->pc = std::make_shared<rtc::PeerConnection>(config);

        // Set up event handlers
        impl_->pc->onStateChange([this](rtc::PeerConnection::State state) {
            LOG_INFO("PeerConnection state changed: " + std::to_string(static_cast<int>(state)));
            if (state == rtc::PeerConnection::State::Connected) {
                impl_->connected = true;
            } else if (state == rtc::PeerConnection::State::Closed || state == rtc::PeerConnection::State::Failed) {
                impl_->connected = false;
            }
            if (impl_->on_state_change) {
                impl_->on_state_change(state == rtc::PeerConnection::State::Connected);
            }
        });

        impl_->pc->onGatheringStateChange([this](rtc::PeerConnection::GatheringState state) {
            LOG_DEBUG("ICE Gathering state: " + std::to_string(static_cast<int>(state)));
            if (state == rtc::PeerConnection::GatheringState::Complete) {
                impl_->gathering_complete = true;
            }
        });

        impl_->pc->onLocalDescription([this](rtc::Description desc) {
            LOG_INFO("Local SDP (" + desc.typeString() + "):\n" + desc.generateSdp());
            std::lock_guard<std::mutex> lock(impl_->mutex);
            impl_->local_sdp = desc.generateSdp();
        });

        impl_->pc->onLocalCandidate([this](const rtc::Candidate& candidate) {
            LOG_DEBUG("Local ICE candidate: " + candidate.candidate());
            if (impl_->on_ice_candidate) {
                impl_->on_ice_candidate(candidate.candidate());
            }
        });

        impl_->pc->onDataChannel([this](std::shared_ptr<rtc::DataChannel> channel) {
            LOG_INFO("DataChannel received");
            std::lock_guard<std::mutex> lock(impl_->mutex);
            impl_->dc = channel;
            // Inline SetupDataChannel logic
            if (impl_->dc) {
                impl_->dc->onOpen([this]() {
                    LOG_INFO("DataChannel open for: " + impl_->peer_id);
                    impl_->connected = true;
                    if (impl_->on_state_change) impl_->on_state_change(true);
                });
                impl_->dc->onClosed([this]() {
                    LOG_INFO("DataChannel closed for: " + impl_->peer_id);
                    impl_->connected = false;
                    if (impl_->on_state_change) impl_->on_state_change(false);
                });
                impl_->dc->onMessage([this](rtc::message_variant data) {
                    if (std::holds_alternative<rtc::binary>(data)) {
                        const auto& bin = std::get<rtc::binary>(data);
                        if (impl_->on_data) impl_->on_data(reinterpret_cast<const uint8_t*>(bin.data()), bin.size());
                    } else if (std::holds_alternative<std::string>(data)) {
                        const auto& str = std::get<std::string>(data);
                        if (impl_->on_data) impl_->on_data(reinterpret_cast<const uint8_t*>(str.data()), str.size());
                    }
                });
            }
        });

        // If this is the offerer, create a data channel immediately
        if (!impl_->dc) {
            impl_->dc = impl_->pc->createDataChannel("data");
            // Inline SetupDataChannel logic
            if (impl_->dc) {
                impl_->dc->onOpen([this]() {
                    LOG_INFO("DataChannel open for: " + impl_->peer_id);
                    impl_->connected = true;
                    if (impl_->on_state_change) impl_->on_state_change(true);
                });
                impl_->dc->onClosed([this]() {
                    LOG_INFO("DataChannel closed for: " + impl_->peer_id);
                    impl_->connected = false;
                    if (impl_->on_state_change) impl_->on_state_change(false);
                });
                impl_->dc->onMessage([this](rtc::message_variant data) {
                    if (std::holds_alternative<rtc::binary>(data)) {
                        const auto& bin = std::get<rtc::binary>(data);
                        if (impl_->on_data) impl_->on_data(reinterpret_cast<const uint8_t*>(bin.data()), bin.size());
                    } else if (std::holds_alternative<std::string>(data)) {
                        const auto& str = std::get<std::string>(data);
                        if (impl_->on_data) impl_->on_data(reinterpret_cast<const uint8_t*>(str.data()), str.size());
                    }
                });
            }
        }

        // QUIC transport (msquic) is not yet supported in this build.
        // TODO: Re-enable msquic integration after clean build.
        impl_->initialized = true;
        LOG_INFO("WebRTCPeerConnection initialized for: " + impl_->peer_id);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to initialize PeerConnection: " + std::string(e.what()));
        return false;
    }
}

// SetupDataChannel removed

void WebRTCPeerConnection::Close() {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (impl_->dc) {
        impl_->dc->close();
        impl_->dc.reset();
    }
    if (impl_->pc) {
        impl_->pc->close();
        impl_->pc.reset();
    }
    // msquic/QUIC transport is disabled in this build
    impl_->connected = false;
    impl_->initialized = false;
    LOG_INFO("WebRTCPeerConnection closed for: " + impl_->peer_id);
}

bool WebRTCPeerConnection::CreateOffer(std::string& sdp_out) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (!impl_->pc) {
        LOG_ERROR("PeerConnection not initialized");
        return false;
    }
    try {
        impl_->pc->setLocalDescription();
        // Wait for gathering to complete (in production, use async/callback)
        int wait_ms = 0;
        while (!impl_->gathering_complete && wait_ms < 5000) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            wait_ms += 10;
        }
        sdp_out = impl_->local_sdp;
        return !sdp_out.empty();
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create offer: " + std::string(e.what()));
        return false;
    }
}

bool WebRTCPeerConnection::CreateAnswer(std::string& sdp_out) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (!impl_->pc) {
        LOG_ERROR("PeerConnection not initialized");
        return false;
    }
    try {
        impl_->pc->setLocalDescription();
        int wait_ms = 0;
        while (!impl_->gathering_complete && wait_ms < 5000) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            wait_ms += 10;
        }
        sdp_out = impl_->local_sdp;
        return !sdp_out.empty();
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create answer: " + std::string(e.what()));
        return false;
    }
}

bool WebRTCPeerConnection::SetRemoteDescription(const std::string& sdp) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (!impl_->pc) {
        LOG_ERROR("PeerConnection not initialized");
        return false;
    }
    try {
        impl_->pc->setRemoteDescription(sdp);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to set remote description: " + std::string(e.what()));
        return false;
    }
}

bool WebRTCPeerConnection::AddIceCandidate(const std::string& candidate) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (!impl_->pc) {
        LOG_ERROR("PeerConnection not initialized");
        return false;
    }
    try {
        impl_->pc->addRemoteCandidate(candidate);
        LOG_DEBUG("Added ICE candidate for: " + impl_->peer_id);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to add ICE candidate: " + std::string(e.what()));
        return false;
    }
}

bool WebRTCPeerConnection::SendData(const uint8_t* data, size_t size) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (!impl_->connected || !impl_->dc || !impl_->dc->isOpen()) {
        LOG_ERROR("Data channel not open for: " + impl_->peer_id);
        return false;
    }
    try {
        rtc::binary bin(reinterpret_cast<const std::byte*>(data), reinterpret_cast<const std::byte*>(data) + size);
        impl_->dc->send(bin);
        LOG_DEBUG("Sent " + std::to_string(size) + " bytes to: " + impl_->peer_id);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to send data: " + std::string(e.what()));
        return false;
    }
}

// --- QUIC Transport API (msquic) removed ---

std::string WebRTCPeerConnection::GetPeerId() const {
    return impl_->peer_id;
}

bool WebRTCPeerConnection::IsConnected() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->connected;
}

void WebRTCPeerConnection::SetOnDataCallback(OnDataCallback callback) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->on_data = callback;
}

void WebRTCPeerConnection::SetOnStateChangeCallback(OnStateChangeCallback callback) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->on_state_change = callback;
}

void WebRTCPeerConnection::SetOnIceCandidateCallback(OnIceCandidateCallback callback) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->on_ice_candidate = callback;
}

void WebRTCPeerConnection::SetOnPacketCallback(OnPacketCallback callback) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->on_packet = callback;
}

void WebRTCPeerConnection::SetPeerPosition(float x, float y, float z) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->pos_x = x;
    impl_->pos_y = y;
    impl_->pos_z = z;
    LOG_DEBUG("SetPeerPosition for " + impl_->peer_id + ": (" +
              std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z) + ")");
}

void WebRTCPeerConnection::GetPeerPosition(float& x, float& y, float& z) const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    x = impl_->pos_x;
    y = impl_->pos_y;
    z = impl_->pos_z;
}

bool WebRTCPeerConnection::IsWithinAOI(float x, float y, float z, float radius) const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    float dx = impl_->pos_x - x;
    float dy = impl_->pos_y - y;
    float dz = impl_->pos_z - z;
    float dist_sq = dx*dx + dy*dy + dz*dz;
    return dist_sq <= radius * radius;
}

void WebRTCPeerConnection::SetPeerScore(float score) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->score = score;
    LOG_DEBUG("SetPeerScore for " + impl_->peer_id + ": " + std::to_string(score));
}

float WebRTCPeerConnection::GetPeerScore() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->score;
}

} // namespace P2P