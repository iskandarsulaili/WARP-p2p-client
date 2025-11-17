#include "../../include/WebRTCPeerConnection.h"
#include "../../include/Logger.h"
#include <api/peer_connection_interface.h>
#include <api/create_peerconnection_factory.h>
#include <api/data_channel_interface.h>
#include <rtc_base/thread.h>
#include <rtc_base/logging.h>
#include <mutex>
#include <memory>

namespace P2P {

struct WebRTCPeerConnection::Impl {
    std::string peer_id;
    rtc::scoped_refptr<rtc::Thread> network_thread;
    rtc::scoped_refptr<rtc::Thread> worker_thread;
    rtc::scoped_refptr<rtc::Thread> signaling_thread;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory;
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc;
    rtc::scoped_refptr<webrtc::DataChannelInterface> dc;

    bool connected = false;
    bool initialized = false;

    // AOI/mesh
    float pos_x = 0.0f, pos_y = 0.0f, pos_z = 0.0f;
    float score = 1.0f;

    OnDataCallback on_data;
    OnStateChangeCallback on_state_change;
    OnIceCandidateCallback on_ice_candidate;
    // Protocol
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
    try {
        // Create threads for libwebrtc
        impl_->network_thread = rtc::Thread::CreateWithSocketServer();
        impl_->worker_thread = rtc::Thread::Create();
        impl_->signaling_thread = rtc::Thread::Create();
        impl_->network_thread->Start();
        impl_->worker_thread->Start();
        impl_->signaling_thread->Start();

        // Create PeerConnectionFactory
        impl_->factory = webrtc::CreatePeerConnectionFactory(
            impl_->network_thread.get(),
            impl_->worker_thread.get(),
            impl_->signaling_thread.get(),
            nullptr, nullptr, nullptr, nullptr
        );
        if (!impl_->factory) {
            LOG_ERROR("Failed to create PeerConnectionFactory");
            return false;
        }

        // Configure ICE servers
        webrtc::PeerConnectionInterface::RTCConfiguration config;

        // Add STUN servers
        for (const auto& server : stun) {
            webrtc::PeerConnectionInterface::IceServer ice_server;
            ice_server.uri = server;
            config.servers.push_back(ice_server);
            LOG_DEBUG("Added STUN server: " + server);
        }

        // Add TURN servers with credentials
        for (const auto& server : turn) {
            webrtc::PeerConnectionInterface::IceServer ice_server;
            ice_server.uri = server;
            if (!turn_username.empty() && !turn_credential.empty()) {
                ice_server.username = turn_username;
                ice_server.password = turn_credential;
                LOG_DEBUG("Added TURN server with credentials: " + server);
            } else {
                LOG_DEBUG("Added TURN server: " + server);
            }
            config.servers.push_back(ice_server);
        }

        // Create peer connection
        webrtc::PeerConnectionDependencies dependencies(this);
        impl_->pc = impl_->factory->CreatePeerConnection(config, std::move(dependencies));
        if (!impl_->pc) {
            LOG_ERROR("Failed to create PeerConnection");
            return false;
        }

        // Set up libwebrtc observer classes for signaling, ICE, and data channel events
        class PeerConnectionObserver : public webrtc::PeerConnectionObserver {
        public:
            PeerConnectionObserver(WebRTCPeerConnection::Impl* impl) : impl_(impl) {}
            void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override {
                LOG_DEBUG("Signaling state changed for: " + impl_->peer_id);
            }
            void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {}
            void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {}
            void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override {
                impl_->dc = data_channel;
                LOG_INFO("Data channel received for: " + impl_->peer_id);
                // Set up data channel observer
                class DataChannelObserver : public webrtc::DataChannelObserver {
                public:
                    DataChannelObserver(WebRTCPeerConnection::Impl* impl) : impl_(impl) {}
                    void OnStateChange() override {
                        LOG_INFO("Data channel state changed for: " + impl_->peer_id);
                    }
                    void OnMessage(const webrtc::DataBuffer& buffer) override {
                        if (impl_->on_data) {
                            impl_->on_data(buffer.data.data(), buffer.data.size());
                        }
                    }
                private:
                    WebRTCPeerConnection::Impl* impl_;
                };
                impl_->dc->RegisterObserver(new DataChannelObserver(impl_));
            }
            void OnRenegotiationNeeded() override {}
            void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override {
                LOG_DEBUG("ICE connection state changed for: " + impl_->peer_id);
            }
            void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override {
                impl_->gathering_complete = (new_state == webrtc::PeerConnectionInterface::kIceGatheringComplete);
                LOG_DEBUG("ICE gathering state changed for: " + impl_->peer_id);
            }
            void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override {
                std::string candidate_str;
                candidate->ToString(&candidate_str);
                if (impl_->on_ice_candidate) {
                    impl_->on_ice_candidate(candidate_str);
                }
                LOG_DEBUG("ICE candidate generated for: " + impl_->peer_id);
            }
        private:
            WebRTCPeerConnection::Impl* impl_;
        };

        impl_->pc->RegisterObserver(new PeerConnectionObserver(impl_.get()));

        impl_->initialized = true;
        LOG_INFO("WebRTCPeerConnection initialized for: " + impl_->peer_id);
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to initialize WebRTCPeerConnection: " + std::string(e.what()));
        return false;
    }
}

void WebRTCPeerConnection::Close() {
    try {
        if (impl_->dc) {
            impl_->dc->close();
            impl_->dc.reset();
        }
        if (impl_->pc) {
            impl_->pc->close();
            impl_->pc.reset();
        }
        impl_->connected = false;
        impl_->initialized = false;
        LOG_INFO("WebRTCPeerConnection closed for: " + impl_->peer_id);
    } catch (const std::exception& e) {
        LOG_ERROR("Error closing WebRTCPeerConnection: " + std::string(e.what()));
    }
}

bool WebRTCPeerConnection::CreateOffer(std::string& sdp_out) {
    try {
        if (!impl_->pc) {
            LOG_ERROR("PeerConnection not initialized");
            return false;
        }

        // Create data channel (this triggers offer creation)
        impl_->dc = impl_->pc->createDataChannel("data");

        // Set up data channel callbacks
        impl_->dc->onOpen([this]() {
            LOG_INFO("Data channel opened for: " + impl_->peer_id);
        });

        impl_->dc->onClosed([this]() {
            LOG_INFO("Data channel closed for: " + impl_->peer_id);
        });

        impl_->dc->onMessage([this](auto data) {
            // Anti-cheat: anomaly detection
            size_t packet_size = 0;
            const uint8_t* packet_data = nullptr;
            if (std::holds_alternative<std::string>(data)) {
                auto& str = std::get<std::string>(data);
                packet_data = reinterpret_cast<const uint8_t*>(str.data());
                packet_size = str.size();
            } else {
                auto& bytes = std::get<rtc::binary>(data);
                packet_data = reinterpret_cast<const uint8_t*>(bytes.data());
                packet_size = bytes.size();
            }
            impl_->total_packet_count++;
            bool suspicious = false;
            // Anti-cheat: check for suspicious packet size
            if (packet_size > 2048 || packet_size == 0) suspicious = true;

            // Anti-cheat: check for valid packet header (first 2 bytes = packet type)
            if (packet_size >= 2) {
                uint16_t packet_type = static_cast<uint16_t>(packet_data[0]) | (static_cast<uint16_t>(packet_data[1]) << 8);
                if (packet_type > 0x0FFF) suspicious = true;
            } else {
                suspicious = true;
            }

            // Anti-cheat: check for repeated patterns (low entropy)
            if (packet_size > 8) {
                size_t repeats = 0;
                for (size_t i = 1; i < packet_size; ++i) {
                    if (packet_data[i] == packet_data[i-1]) repeats++;
                }
                if (repeats > packet_size / 2) suspicious = true;
            }

            if (suspicious) {
                impl_->suspicious_packet_count++;
                LOG_WARN("AntiCheat: Suspicious packet detected from peer " + impl_->peer_id +
                         " size=" + std::to_string(packet_size));
            }
            // Periodically update anomaly score and prune if needed
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - impl_->last_anomaly_check).count() > 10) {
                float ratio = impl_->total_packet_count > 0
                    ? static_cast<float>(impl_->suspicious_packet_count) / impl_->total_packet_count
                    : 0.0f;
                impl_->anomaly_score = ratio;
                LOG_DEBUG("AntiCheat: Updated anomaly score for peer " + impl_->peer_id +
                          " score=" + std::to_string(impl_->anomaly_score));
                if (impl_->anomaly_score > 0.2f) {
                    LOG_WARN("AntiCheat: Peer " + impl_->peer_id + " flagged for pruning (anomaly score=" +
                             std::to_string(impl_->anomaly_score) + ")");
                    SetPeerScore(0.0f); // Prune in mesh refresh
                }
                impl_->last_anomaly_check = now;
                impl_->suspicious_packet_count = 0;
                impl_->total_packet_count = 0;
            }
            // Normal protocol handling
            if (impl_->on_data) {
                impl_->on_data(packet_data, packet_size);
                if (impl_->on_packet) impl_->on_packet(packet_data, packet_size);
            }
        });

        // Wait for local description
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::lock_guard<std::mutex> lock(impl_->mutex);
        sdp_out = impl_->local_sdp;

        LOG_INFO("Created offer for: " + impl_->peer_id);
        return !sdp_out.empty();

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create offer: " + std::string(e.what()));
        return false;
    }
}

bool WebRTCPeerConnection::CreateAnswer(std::string& sdp_out) {
    try {
        if (!impl_->pc) {
            LOG_ERROR("PeerConnection not initialized");
            return false;
        }

        // Wait for local description (answer is created automatically after setRemoteDescription)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::lock_guard<std::mutex> lock(impl_->mutex);
        sdp_out = impl_->local_sdp;

        LOG_INFO("Created answer for: " + impl_->peer_id);
        return !sdp_out.empty();

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create answer: " + std::string(e.what()));
        return false;
    }
}

bool WebRTCPeerConnection::SetRemoteDescription(const std::string& sdp) {
    try {
        if (!impl_->pc) {
            LOG_ERROR("PeerConnection not initialized");
            return false;
        }

        rtc::Description description(sdp, rtc::Description::Type::Unspec);
        impl_->pc->setRemoteDescription(description);

        // Set up data channel callback if we're the answerer
        if (!impl_->dc) {
            impl_->pc->onDataChannel([this](std::shared_ptr<rtc::DataChannel> dc) {
                impl_->dc = dc;

                dc->onOpen([this]() {
                    LOG_INFO("Data channel opened for: " + impl_->peer_id);
                });

                dc->onClosed([this]() {
                    LOG_INFO("Data channel closed for: " + impl_->peer_id);
                });

                dc->onMessage([this](auto data) {
                    if (impl_->on_data) {
                        if (std::holds_alternative<std::string>(data)) {
                            auto& str = std::get<std::string>(data);
                            impl_->on_data(reinterpret_cast<const uint8_t*>(str.data()), str.size());
                        } else {
                            auto& bytes = std::get<rtc::binary>(data);
                            impl_->on_data(reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size());
                        }
                    }
                });
            });
        }

        LOG_INFO("Set remote description for: " + impl_->peer_id);
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to set remote description: " + std::string(e.what()));
        return false;
    }
}

bool WebRTCPeerConnection::AddIceCandidate(const std::string& candidate) {
    try {
        if (!impl_->pc) {
            LOG_ERROR("PeerConnection not initialized");
            return false;
        }

        rtc::Candidate cand(candidate, "");
        impl_->pc->addRemoteCandidate(cand);

        LOG_DEBUG("Added ICE candidate for: " + impl_->peer_id);
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to add ICE candidate: " + std::string(e.what()));
        return false;
    }
}

bool WebRTCPeerConnection::SendData(const uint8_t* data, size_t size) {
    try {
        if (!impl_->connected || !impl_->dc || !impl_->dc->isOpen()) {
            LOG_ERROR("Data channel not open for: " + impl_->peer_id);
            return false;
        }

        // Convert uint8_t* to std::byte vector
        rtc::binary binary_data;
        binary_data.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            binary_data.push_back(static_cast<std::byte>(data[i]));
        }
        impl_->dc->send(binary_data);

        LOG_DEBUG("Sent " + std::to_string(size) + " bytes to: " + impl_->peer_id);
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to send data: " + std::string(e.what()));
        return false;
    }
}

bool WebRTCPeerConnection::IsConnected() const {
    return impl_->connected && impl_->dc && impl_->dc->isOpen();
}

std::string WebRTCPeerConnection::GetPeerId() const {
    return impl_->peer_id;
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

void WebRTCPeerConnection::SetOnPacketCallback(OnPacketCallback callback) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->on_packet = callback;
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
