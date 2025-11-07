#include "WebRTCPeerConnection.hpp"
#include <iostream>
#include <sstream>
#include <cstring>

// In production, include actual WebRTC headers
// #include <api/create_peerconnection_factory.h>
// #include <api/audio_codecs/builtin_audio_decoder_factory.h>
// #include <api/audio_codecs/builtin_audio_encoder_factory.h>
// #include <api/video_codecs/builtin_video_decoder_factory.h>
// #include <api/video_codecs/builtin_video_encoder_factory.h>
// #include <api/peer_connection_interface.h>
// #include <api/data_channel_interface.h>

WebRTCPeerConnection::WebRTCPeerConnection(const std::string& peer_id)
    : peer_id_(peer_id)
    , state_(State::NEW)
    , data_channel_state_(DataChannelState::CLOSED)
    , pc_factory_(nullptr)
    , peer_connection_(nullptr)
    , data_channel_(nullptr)
{
    log_info("WebRTC peer connection created for peer: " + peer_id_);
    
    // Initialize statistics
    stats_.bytes_sent = 0;
    stats_.bytes_received = 0;
    stats_.packets_sent = 0;
    stats_.packets_received = 0;
    stats_.current_round_trip_time_ms = 0;
    stats_.available_outgoing_bitrate = 0;
}

WebRTCPeerConnection::~WebRTCPeerConnection() {
    close();
    
    // Clean up WebRTC resources
    if (data_channel_) {
        // data_channel_->Release();
        data_channel_ = nullptr;
    }
    
    if (peer_connection_) {
        // peer_connection_->Release();
        peer_connection_ = nullptr;
    }
    
    if (pc_factory_) {
        // pc_factory_->Release();
        pc_factory_ = nullptr;
    }
    
    log_info("WebRTC peer connection destroyed for peer: " + peer_id_);
}

bool WebRTCPeerConnection::initialize(const std::vector<std::string>& stun_servers,
                                     const std::vector<std::string>& turn_servers) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    log_info("Initializing WebRTC peer connection");
    
    try {
        // In production, create actual WebRTC peer connection factory
        // rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pc_factory =
        //     webrtc::CreatePeerConnectionFactory(
        //         nullptr, nullptr, nullptr,
        //         nullptr,
        //         webrtc::CreateBuiltinAudioEncoderFactory(),
        //         webrtc::CreateBuiltinAudioDecoderFactory(),
        //         webrtc::CreateBuiltinVideoEncoderFactory(),
        //         webrtc::CreateBuiltinVideoDecoderFactory(),
        //         nullptr, nullptr
        //     );
        
        // Configure ICE servers
        // webrtc::PeerConnectionInterface::RTCConfiguration config;
        // for (const auto& stun : stun_servers) {
        //     webrtc::PeerConnectionInterface::IceServer server;
        //     server.uri = stun;
        //     config.servers.push_back(server);
        // }
        // for (const auto& turn : turn_servers) {
        //     webrtc::PeerConnectionInterface::IceServer server;
        //     server.uri = turn;
        //     config.servers.push_back(server);
        // }
        
        // Create peer connection
        // peer_connection_ = pc_factory->CreatePeerConnection(config, ...);
        
        log_info("WebRTC peer connection initialized successfully");
        log_info("STUN servers: " + std::to_string(stun_servers.size()));
        log_info("TURN servers: " + std::to_string(turn_servers.size()));
        
        set_state(State::NEW);
        return true;
        
    } catch (const std::exception& e) {
        log_error("Failed to initialize WebRTC: " + std::string(e.what()));
        if (error_callback_) {
            error_callback_("Initialization failed: " + std::string(e.what()));
        }
        return false;
    }
}

bool WebRTCPeerConnection::create_offer() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    log_info("Creating WebRTC offer");
    
    if (!peer_connection_) {
        log_error("Peer connection not initialized");
        return false;
    }
    
    try {
        // Create data channel before creating offer
        create_data_channel();
        
        // In production, create actual offer
        // webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
        // peer_connection_->CreateOffer(observer, options);
        
        // For reference implementation, simulate offer creation
        json offer = {
            {"type", "offer"},
            {"sdp", "v=0\r\no=- 123456789 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\n"
                   "a=group:BUNDLE 0\r\na=msid-semantic: WMS\r\n"
                   "m=application 9 UDP/DTLS/SCTP webrtc-datachannel\r\n"
                   "c=IN IP4 0.0.0.0\r\na=ice-ufrag:abcd\r\na=ice-pwd:1234567890\r\n"
                   "a=fingerprint:sha-256 AA:BB:CC:DD:EE:FF:00:11:22:33:44:55:66:77:88:99:AA:BB:CC:DD:EE:FF:00:11:22:33:44:55:66:77:88:99\r\n"
                   "a=setup:actpass\r\na=mid:0\r\na=sctp-port:5000\r\na=max-message-size:262144\r\n"}
        };
        
        log_info("Offer created successfully");
        
        if (offer_callback_) {
            offer_callback_(offer);
        }
        
        set_state(State::CONNECTING);
        return true;
        
    } catch (const std::exception& e) {
        log_error("Failed to create offer: " + std::string(e.what()));
        if (error_callback_) {
            error_callback_("Offer creation failed: " + std::string(e.what()));
        }
        return false;
    }
}

bool WebRTCPeerConnection::create_answer() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    log_info("Creating WebRTC answer");
    
    if (!peer_connection_) {
        log_error("Peer connection not initialized");
        return false;
    }
    
    try {
        // In production, create actual answer
        // webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
        // peer_connection_->CreateAnswer(observer, options);
        
        // For reference implementation, simulate answer creation
        json answer = {
            {"type", "answer"},
            {"sdp", "v=0\r\no=- 987654321 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\n"
                   "a=group:BUNDLE 0\r\na=msid-semantic: WMS\r\n"
                   "m=application 9 UDP/DTLS/SCTP webrtc-datachannel\r\n"
                   "c=IN IP4 0.0.0.0\r\na=ice-ufrag:efgh\r\na=ice-pwd:0987654321\r\n"
                   "a=fingerprint:sha-256 FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00:FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00\r\n"
                   "a=setup:active\r\na=mid:0\r\na=sctp-port:5000\r\na=max-message-size:262144\r\n"}
        };
        
        log_info("Answer created successfully");
        
        if (answer_callback_) {
            answer_callback_(answer);
        }
        
        set_state(State::CONNECTING);
        return true;
        
    } catch (const std::exception& e) {
        log_error("Failed to create answer: " + std::string(e.what()));
        if (error_callback_) {
            error_callback_("Answer creation failed: " + std::string(e.what()));
        }
        return false;
    }
}

bool WebRTCPeerConnection::set_remote_sdp(const json& sdp) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string type = sdp["type"];
    log_info("Setting remote SDP: " + type);
    
    if (!peer_connection_) {
        log_error("Peer connection not initialized");
        return false;
    }
    
    try {
        // In production, set actual remote SDP
        // std::string sdp_str = sdp["sdp"];
        // webrtc::SdpParseError error;
        // std::unique_ptr<webrtc::SessionDescriptionInterface> session_description =
        //     webrtc::CreateSessionDescription(type, sdp_str, &error);
        // peer_connection_->SetRemoteDescription(observer, session_description.release());
        
        log_info("Remote SDP set successfully");
        return true;
        
    } catch (const std::exception& e) {
        log_error("Failed to set remote SDP: " + std::string(e.what()));
        if (error_callback_) {
            error_callback_("Set remote SDP failed: " + std::string(e.what()));
        }
        return false;
    }
}

bool WebRTCPeerConnection::add_ice_candidate(const json& candidate) {
    std::lock_guard<std::mutex> lock(mutex_);

    log_info("Adding ICE candidate");

    if (!peer_connection_) {
        log_error("Peer connection not initialized");
        return false;
    }

    try {
        // In production, add actual ICE candidate
        // std::string sdp_mid = candidate["sdpMid"];
        // int sdp_mline_index = candidate["sdpMLineIndex"];
        // std::string sdp = candidate["candidate"];
        //
        // webrtc::SdpParseError error;
        // std::unique_ptr<webrtc::IceCandidateInterface> ice_candidate(
        //     webrtc::CreateIceCandidate(sdp_mid, sdp_mline_index, sdp, &error)
        // );
        // peer_connection_->AddIceCandidate(ice_candidate.get());

        log_info("ICE candidate added successfully");
        return true;

    } catch (const std::exception& e) {
        log_error("Failed to add ICE candidate: " + std::string(e.what()));
        if (error_callback_) {
            error_callback_("Add ICE candidate failed: " + std::string(e.what()));
        }
        return false;
    }
}

bool WebRTCPeerConnection::send_data(const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!data_channel_ || data_channel_state_ != DataChannelState::OPEN) {
        log_error("Data channel not open");
        return false;
    }

    try {
        // In production, send actual data
        // webrtc::DataBuffer buffer(rtc::CopyOnWriteBuffer(data.data(), data.size()), true);
        // data_channel_->Send(buffer);

        stats_.bytes_sent += data.size();
        stats_.packets_sent++;

        log_info("Sent " + std::to_string(data.size()) + " bytes");
        return true;

    } catch (const std::exception& e) {
        log_error("Failed to send data: " + std::string(e.what()));
        if (error_callback_) {
            error_callback_("Send data failed: " + std::string(e.what()));
        }
        return false;
    }
}

bool WebRTCPeerConnection::send_message(const std::string& message) {
    std::vector<uint8_t> data(message.begin(), message.end());
    return send_data(data);
}

void WebRTCPeerConnection::close() {
    std::lock_guard<std::mutex> lock(mutex_);

    log_info("Closing peer connection");

    if (data_channel_) {
        // data_channel_->Close();
        set_data_channel_state(DataChannelState::CLOSED);
    }

    if (peer_connection_) {
        // peer_connection_->Close();
    }

    set_state(State::CLOSED);
}

WebRTCPeerConnection::State WebRTCPeerConnection::get_state() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

WebRTCPeerConnection::DataChannelState WebRTCPeerConnection::get_data_channel_state() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return data_channel_state_;
}

WebRTCPeerConnection::Statistics WebRTCPeerConnection::get_statistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void WebRTCPeerConnection::set_state(State new_state) {
    if (state_ != new_state) {
        state_ = new_state;
        log_info("State changed to: " + std::to_string(static_cast<int>(new_state)));

        if (state_callback_) {
            state_callback_(new_state);
        }
    }
}

void WebRTCPeerConnection::set_data_channel_state(DataChannelState new_state) {
    if (data_channel_state_ != new_state) {
        data_channel_state_ = new_state;
        log_info("Data channel state changed to: " + std::to_string(static_cast<int>(new_state)));

        if (new_state == DataChannelState::OPEN) {
            set_state(State::CONNECTED);
            if (data_channel_open_callback_) {
                data_channel_open_callback_();
            }
        } else if (new_state == DataChannelState::CLOSED) {
            if (data_channel_close_callback_) {
                data_channel_close_callback_();
            }
        }
    }
}

void WebRTCPeerConnection::handle_ice_candidate(const webrtc::IceCandidateInterface* candidate) {
    if (!candidate) return;

    // In production, serialize ICE candidate
    // std::string sdp;
    // candidate->ToString(&sdp);
    //
    // json ice_json = {
    //     {"candidate", sdp},
    //     {"sdpMid", candidate->sdp_mid()},
    //     {"sdpMLineIndex", candidate->sdp_mline_index()}
    // };

    json ice_json = {
        {"candidate", "candidate:1 1 UDP 2130706431 192.168.1.100 54321 typ host"},
        {"sdpMid", "0"},
        {"sdpMLineIndex", 0}
    };

    log_info("ICE candidate generated");

    if (ice_candidate_callback_) {
        ice_candidate_callback_(ice_json);
    }
}

void WebRTCPeerConnection::handle_data_channel_state_change() {
    // In production, get actual data channel state
    // auto state = data_channel_->state();
    //
    // switch (state) {
    //     case webrtc::DataChannelInterface::kConnecting:
    //         set_data_channel_state(DataChannelState::CONNECTING);
    //         break;
    //     case webrtc::DataChannelInterface::kOpen:
    //         set_data_channel_state(DataChannelState::OPEN);
    //         break;
    //     case webrtc::DataChannelInterface::kClosing:
    //         set_data_channel_state(DataChannelState::CLOSING);
    //         break;
    //     case webrtc::DataChannelInterface::kClosed:
    //         set_data_channel_state(DataChannelState::CLOSED);
    //         break;
    // }

    // For reference implementation
    set_data_channel_state(DataChannelState::OPEN);
}

void WebRTCPeerConnection::handle_data_channel_message(const uint8_t* data, size_t size) {
    stats_.bytes_received += size;
    stats_.packets_received++;

    log_info("Received " + std::to_string(size) + " bytes");

    if (message_callback_) {
        std::vector<uint8_t> message_data(data, data + size);
        message_callback_(message_data);
    }
}

void WebRTCPeerConnection::create_data_channel() {
    if (data_channel_) {
        log_info("Data channel already exists");
        return;
    }

    // In production, create actual data channel
    // webrtc::DataChannelInit config;
    // config.ordered = true;
    // config.reliable = true;
    //
    // data_channel_ = peer_connection_->CreateDataChannel("game-data", &config);

    log_info("Data channel created");
    set_data_channel_state(DataChannelState::CONNECTING);
}

void WebRTCPeerConnection::log_info(const std::string& message) {
    std::cout << "[WebRTC] [" << peer_id_ << "] INFO: " << message << std::endl;
}

void WebRTCPeerConnection::log_error(const std::string& message) {
    std::cerr << "[WebRTC] [" << peer_id_ << "] ERROR: " << message << std::endl;
}

