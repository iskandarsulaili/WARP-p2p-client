#include "../../include/WebRTCPeerConnection.h"
#include "../../include/Logger.h"
#include <rtc/rtc.hpp>
#include <mutex>
#include <memory>
#include <condition_variable>

namespace P2P {

struct WebRTCPeerConnection::Impl {
    std::string peer_id;
    std::shared_ptr<rtc::PeerConnection> pc;
    std::shared_ptr<rtc::DataChannel> dc;

    bool connected = false;
    bool initialized = false;

    OnDataCallback on_data;
    OnStateChangeCallback on_state_change;
    OnIceCandidateCallback on_ice_candidate;

    std::mutex mutex;
    std::condition_variable cv;
    std::string local_sdp;
    bool sdp_ready = false;
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

bool WebRTCPeerConnection::Initialize(const std::vector<std::string>& stun, const std::vector<std::string>& turn) {
    try {
        // Configure ICE servers
        rtc::Configuration config;

        // Add STUN servers
        for (const auto& server : stun) {
            config.iceServers.emplace_back(server);
            LOG_DEBUG("Added STUN server: " + server);
        }

        // Add TURN servers
        for (const auto& server : turn) {
            config.iceServers.emplace_back(server);
            LOG_DEBUG("Added TURN server: " + server);
        }

        // Create peer connection
        impl_->pc = std::make_shared<rtc::PeerConnection>(config);

        // Set up callbacks
        impl_->pc->onLocalDescription([this](rtc::Description description) {
            std::lock_guard<std::mutex> lock(impl_->mutex);
            impl_->local_sdp = std::string(description);
            impl_->sdp_ready = true;
            impl_->cv.notify_one();
            LOG_DEBUG("Local description created for: " + impl_->peer_id);
        });

        impl_->pc->onLocalCandidate([this](rtc::Candidate candidate) {
            std::lock_guard<std::mutex> lock(impl_->mutex);
            if (impl_->on_ice_candidate) {
                impl_->on_ice_candidate(std::string(candidate));
            }
            LOG_DEBUG("ICE candidate generated for: " + impl_->peer_id);
        });

        impl_->pc->onStateChange([this](rtc::PeerConnection::State state) {
            bool was_connected = impl_->connected;
            impl_->connected = (state == rtc::PeerConnection::State::Connected);

            LOG_INFO("Connection state changed for " + impl_->peer_id + ": " +
                     std::to_string(static_cast<int>(state)));

            if (impl_->on_state_change && was_connected != impl_->connected) {
                impl_->on_state_change(impl_->connected);
            }
        });

        impl_->pc->onGatheringStateChange([this](rtc::PeerConnection::GatheringState state) {
            impl_->gathering_complete = (state == rtc::PeerConnection::GatheringState::Complete);
            LOG_DEBUG("Gathering state changed for " + impl_->peer_id + ": " +
                     std::to_string(static_cast<int>(state)));
        });

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

        // Wait for local description with timeout (5 seconds)
        std::unique_lock<std::mutex> lock(impl_->mutex);
        if (!impl_->cv.wait_for(lock, std::chrono::seconds(5), [this] { return impl_->sdp_ready; })) {
            LOG_ERROR("Timeout waiting for local description (offer) for: " + impl_->peer_id);
            return false;
        }

        sdp_out = impl_->local_sdp;
        impl_->sdp_ready = false;  // Reset for next use

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
        std::unique_lock<std::mutex> lock(impl_->mutex);
        if (!impl_->cv.wait_for(lock, std::chrono::seconds(5), [this] { return impl_->sdp_ready; })) {
            LOG_ERROR("Timeout waiting for local description (answer) for: " + impl_->peer_id);
            return false;
        }

        sdp_out = impl_->local_sdp;
        impl_->sdp_ready = false;  // Reset for next use

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

} // namespace P2P
