#include "WebRTCManager.hpp"
#include <iostream>
#include <algorithm>

WebRTCManager::WebRTCManager()
    : p2p_network_(nullptr)
{
    log_info("WebRTC Manager created");
    
    // Default bandwidth configuration
    bandwidth_config_.max_bitrate_kbps = 2500;
    bandwidth_config_.min_bitrate_kbps = 300;
    bandwidth_config_.start_bitrate_kbps = 1000;
    bandwidth_config_.max_packet_size_bytes = 1200;
    bandwidth_config_.enable_congestion_control = true;
}

WebRTCManager::~WebRTCManager() {
    shutdown();
    log_info("WebRTC Manager destroyed");
}

bool WebRTCManager::initialize(P2PNetwork* p2p_network,
                               const std::vector<std::string>& stun_servers,
                               const std::vector<std::string>& turn_servers,
                               const BandwidthConfig& bandwidth_config) {
    if (!p2p_network) {
        log_error("P2P network is null");
        return false;
    }
    
    p2p_network_ = p2p_network;
    stun_servers_ = stun_servers;
    turn_servers_ = turn_servers;
    bandwidth_config_ = bandwidth_config;
    
    log_info("Initializing WebRTC Manager");
    log_info("STUN servers: " + std::to_string(stun_servers_.size()));
    log_info("TURN servers: " + std::to_string(turn_servers_.size()));
    log_info("Max bitrate: " + std::to_string(bandwidth_config_.max_bitrate_kbps) + " kbps");
    
    // Set up P2P network callbacks
    setup_p2p_callbacks();
    
    log_info("WebRTC Manager initialized successfully");
    return true;
}

void WebRTCManager::shutdown() {
    log_info("Shutting down WebRTC Manager");
    
    std::lock_guard<std::mutex> lock(peers_mutex_);
    
    // Close all peer connections
    for (auto& pair : peer_connections_) {
        pair.second->close();
    }
    
    peer_connections_.clear();
    p2p_network_ = nullptr;
}

void WebRTCManager::setup_p2p_callbacks() {
    if (!p2p_network_) return;
    
    // Handle incoming offers
    p2p_network_->on_offer_received([this](const std::string& from_peer, const json& sdp) {
        handle_offer_received(from_peer, sdp);
    });
    
    // Handle incoming answers
    p2p_network_->on_answer_received([this](const std::string& from_peer, const json& sdp) {
        handle_answer_received(from_peer, sdp);
    });
    
    // Handle incoming ICE candidates
    p2p_network_->on_ice_candidate_received([this](const std::string& from_peer, const json& candidate) {
        handle_ice_candidate_received(from_peer, candidate);
    });
    
    // Handle peer joined
    p2p_network_->on_peer_joined([this](const std::string& peer_id) {
        handle_peer_joined(peer_id);
    });
    
    // Handle peer left
    p2p_network_->on_peer_left([this](const std::string& peer_id) {
        handle_peer_left(peer_id);
    });
}

bool WebRTCManager::connect_to_peer(const std::string& peer_id) {
    log_info("Connecting to peer: " + peer_id);
    
    auto* peer_conn = get_or_create_peer_connection(peer_id);
    if (!peer_conn) {
        log_error("Failed to create peer connection for: " + peer_id);
        return false;
    }
    
    // Create offer and send via signaling
    if (!peer_conn->create_offer()) {
        log_error("Failed to create offer for: " + peer_id);
        return false;
    }
    
    return true;
}

void WebRTCManager::disconnect_from_peer(const std::string& peer_id) {
    log_info("Disconnecting from peer: " + peer_id);
    remove_peer_connection(peer_id);
}

bool WebRTCManager::send_to_peer(const std::string& peer_id, const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(peers_mutex_);
    
    auto it = peer_connections_.find(peer_id);
    if (it == peer_connections_.end()) {
        log_error("Peer not found: " + peer_id);
        return false;
    }
    
    if (!it->second->is_data_channel_open()) {
        log_error("Data channel not open for peer: " + peer_id);
        return false;
    }
    
    // Apply bandwidth management
    if (bandwidth_config_.enable_congestion_control) {
        if (data.size() > bandwidth_config_.max_packet_size_bytes) {
            log_error("Packet size exceeds maximum: " + std::to_string(data.size()));
            return false;
        }
    }
    
    return it->second->send_data(data);
}

bool WebRTCManager::send_message_to_peer(const std::string& peer_id, const std::string& message) {
    std::vector<uint8_t> data(message.begin(), message.end());
    return send_to_peer(peer_id, data);
}

size_t WebRTCManager::broadcast(const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(peers_mutex_);
    
    size_t sent_count = 0;
    
    for (auto& pair : peer_connections_) {
        if (pair.second->is_data_channel_open()) {
            if (pair.second->send_data(data)) {
                sent_count++;
            }
        }
    }
    
    log_info("Broadcast to " + std::to_string(sent_count) + " peers");
    return sent_count;
}

std::vector<std::string> WebRTCManager::get_connected_peers() const {
    std::lock_guard<std::mutex> lock(peers_mutex_);
    
    std::vector<std::string> connected_peers;
    
    for (const auto& pair : peer_connections_) {
        if (pair.second->is_connected()) {
            connected_peers.push_back(pair.first);
        }
    }
    
    return connected_peers;
}

std::unique_ptr<WebRTCManager::PeerInfo> WebRTCManager::get_peer_info(const std::string& peer_id) const {
    std::lock_guard<std::mutex> lock(peers_mutex_);
    
    auto it = peer_connections_.find(peer_id);
    if (it == peer_connections_.end()) {
        return nullptr;
    }
    
    auto info = std::make_unique<PeerInfo>();
    info->peer_id = peer_id;
    info->state = it->second->get_state();
    info->data_channel_state = it->second->get_data_channel_state();
    
    auto stats = it->second->get_statistics();
    info->bytes_sent = stats.bytes_sent;
    info->bytes_received = stats.bytes_received;
    info->latency_ms = stats.current_round_trip_time_ms;

    return info;
}

std::vector<WebRTCManager::PeerInfo> WebRTCManager::get_all_peer_info() const {
    std::lock_guard<std::mutex> lock(peers_mutex_);

    std::vector<PeerInfo> all_info;

    for (const auto& pair : peer_connections_) {
        PeerInfo info;
        info.peer_id = pair.first;
        info.state = pair.second->get_state();
        info.data_channel_state = pair.second->get_data_channel_state();

        auto stats = pair.second->get_statistics();
        info.bytes_sent = stats.bytes_sent;
        info.bytes_received = stats.bytes_received;
        info.latency_ms = stats.current_round_trip_time_ms;

        all_info.push_back(info);
    }

    return all_info;
}

bool WebRTCManager::is_connected_to_peer(const std::string& peer_id) const {
    std::lock_guard<std::mutex> lock(peers_mutex_);

    auto it = peer_connections_.find(peer_id);
    if (it == peer_connections_.end()) {
        return false;
    }

    return it->second->is_connected();
}

size_t WebRTCManager::get_peer_count() const {
    std::lock_guard<std::mutex> lock(peers_mutex_);

    size_t count = 0;
    for (const auto& pair : peer_connections_) {
        if (pair.second->is_connected()) {
            count++;
        }
    }

    return count;
}

void WebRTCManager::update() {
    // Called from main game loop
    // Process any pending events or cleanup

    std::lock_guard<std::mutex> lock(peers_mutex_);

    // Remove disconnected peers
    std::vector<std::string> to_remove;

    for (const auto& pair : peer_connections_) {
        auto state = pair.second->get_state();
        if (state == WebRTCPeerConnection::State::FAILED ||
            state == WebRTCPeerConnection::State::CLOSED) {
            to_remove.push_back(pair.first);
        }
    }

    for (const auto& peer_id : to_remove) {
        log_info("Removing disconnected peer: " + peer_id);
        peer_connections_.erase(peer_id);

        if (peer_disconnected_callback_) {
            peer_disconnected_callback_(peer_id);
        }
    }
}

void WebRTCManager::handle_offer_received(const std::string& from_peer, const json& sdp) {
    log_info("Received offer from: " + from_peer);

    auto* peer_conn = get_or_create_peer_connection(from_peer);
    if (!peer_conn) {
        log_error("Failed to create peer connection for: " + from_peer);
        return;
    }

    // Set remote SDP
    if (!peer_conn->set_remote_sdp(sdp)) {
        log_error("Failed to set remote SDP from: " + from_peer);
        return;
    }

    // Create answer
    if (!peer_conn->create_answer()) {
        log_error("Failed to create answer for: " + from_peer);
        return;
    }
}

void WebRTCManager::handle_answer_received(const std::string& from_peer, const json& sdp) {
    log_info("Received answer from: " + from_peer);

    std::lock_guard<std::mutex> lock(peers_mutex_);

    auto it = peer_connections_.find(from_peer);
    if (it == peer_connections_.end()) {
        log_error("Peer connection not found for: " + from_peer);
        return;
    }

    // Set remote SDP
    if (!it->second->set_remote_sdp(sdp)) {
        log_error("Failed to set remote SDP from: " + from_peer);
        return;
    }
}

void WebRTCManager::handle_ice_candidate_received(const std::string& from_peer, const json& candidate) {
    log_info("Received ICE candidate from: " + from_peer);

    std::lock_guard<std::mutex> lock(peers_mutex_);

    auto it = peer_connections_.find(from_peer);
    if (it == peer_connections_.end()) {
        log_error("Peer connection not found for: " + from_peer);
        return;
    }

    // Add ICE candidate
    if (!it->second->add_ice_candidate(candidate)) {
        log_error("Failed to add ICE candidate from: " + from_peer);
        return;
    }
}

void WebRTCManager::handle_peer_joined(const std::string& peer_id) {
    log_info("Peer joined: " + peer_id);

    // Automatically initiate connection to new peer
    connect_to_peer(peer_id);
}

void WebRTCManager::handle_peer_left(const std::string& peer_id) {
    log_info("Peer left: " + peer_id);

    // Remove peer connection
    remove_peer_connection(peer_id);
}

WebRTCPeerConnection* WebRTCManager::get_or_create_peer_connection(const std::string& peer_id) {
    std::lock_guard<std::mutex> lock(peers_mutex_);

    auto it = peer_connections_.find(peer_id);
    if (it != peer_connections_.end()) {
        return it->second.get();
    }

    // Create new peer connection
    auto peer_conn = std::make_unique<WebRTCPeerConnection>(peer_id);

    // Initialize with STUN/TURN servers
    if (!peer_conn->initialize(stun_servers_, turn_servers_)) {
        log_error("Failed to initialize peer connection for: " + peer_id);
        return nullptr;
    }

    // Set up callbacks
    peer_conn->on_state_change([this, peer_id](WebRTCPeerConnection::State state) {
        if (state == WebRTCPeerConnection::State::CONNECTED) {
            log_info("Peer connected: " + peer_id);
            if (peer_connected_callback_) {
                peer_connected_callback_(peer_id);
            }
        } else if (state == WebRTCPeerConnection::State::DISCONNECTED ||
                   state == WebRTCPeerConnection::State::FAILED) {
            log_info("Peer disconnected: " + peer_id);
            if (peer_disconnected_callback_) {
                peer_disconnected_callback_(peer_id);
            }
        }
    });

    peer_conn->on_message_received([this, peer_id](const std::vector<uint8_t>& data) {
        if (message_received_callback_) {
            message_received_callback_(peer_id, data);
        }
    });

    peer_conn->on_ice_candidate([this, peer_id](const json& candidate) {
        if (p2p_network_) {
            p2p_network_->send_ice_candidate(peer_id, candidate);
        }
    });

    peer_conn->on_offer_created([this, peer_id](const json& sdp) {
        if (p2p_network_) {
            p2p_network_->send_offer(peer_id, sdp);
        }
    });

    peer_conn->on_answer_created([this, peer_id](const json& sdp) {
        if (p2p_network_) {
            p2p_network_->send_answer(peer_id, sdp);
        }
    });

    peer_conn->on_error([this, peer_id](const std::string& error) {
        log_error("Peer error [" + peer_id + "]: " + error);
        if (error_callback_) {
            error_callback_("Peer " + peer_id + ": " + error);
        }
    });

    auto* ptr = peer_conn.get();
    peer_connections_[peer_id] = std::move(peer_conn);

    return ptr;
}

void WebRTCManager::remove_peer_connection(const std::string& peer_id) {
    std::lock_guard<std::mutex> lock(peers_mutex_);

    auto it = peer_connections_.find(peer_id);
    if (it != peer_connections_.end()) {
        it->second->close();
        peer_connections_.erase(it);

        if (peer_disconnected_callback_) {
            peer_disconnected_callback_(peer_id);
        }
    }
}

void WebRTCManager::log_info(const std::string& message) {
    std::cout << "[WebRTCManager] INFO: " << message << std::endl;
}

void WebRTCManager::log_error(const std::string& message) {
    std::cerr << "[WebRTCManager] ERROR: " << message << std::endl;
}

