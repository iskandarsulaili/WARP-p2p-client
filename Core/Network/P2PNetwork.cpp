#include "P2PNetwork.hpp"
#include <iostream>
#include <sstream>

P2PNetwork::P2PNetwork() : connected_(false) {
    // Initialize WebSocket client
    ws_client_.init_asio();
    ws_client_.set_access_channels(websocketpp::log::alevel::none);
    ws_client_.set_error_channels(websocketpp::log::elevel::all);
    
    // Set up event handlers
    ws_client_.set_open_handler(bind(&P2PNetwork::on_open, this, ::_1));
    ws_client_.set_close_handler(bind(&P2PNetwork::on_close, this, ::_1));
    ws_client_.set_fail_handler(bind(&P2PNetwork::on_fail, this, ::_1));
    ws_client_.set_message_handler(bind(&P2PNetwork::on_message, this, ::_1, ::_2));
    
    // Set TLS init handler for WSS support
    ws_client_.set_tls_init_handler([](connection_hdl) {
        return websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::tlsv12);
    });
}

P2PNetwork::~P2PNetwork() {
    disconnect();
}

bool P2PNetwork::connect(const std::string& url, const std::string& peer_id, 
                         const std::string& session_id) {
    peer_id_ = peer_id;
    session_id_ = session_id;
    
    try {
        std::string connection_url = build_connection_url(url, peer_id, session_id);
        log_info("Connecting to P2P coordinator: " + connection_url);
        
        websocketpp::lib::error_code ec;
        WebSocketClient::connection_ptr con = ws_client_.get_connection(connection_url, ec);
        
        if (ec) {
            log_error("Connection creation failed: " + ec.message());
            return false;
        }
        
        ws_connection_ = con->get_handle();
        ws_client_.connect(con);
        
        // Run WebSocket client in separate thread
        ws_thread_ = std::thread([this]() {
            ws_client_.run();
        });
        
        return true;
    } catch (const std::exception& e) {
        log_error("Connection exception: " + std::string(e.what()));
        return false;
    }
}

void P2PNetwork::disconnect() {
    if (!connected_) return;
    
    try {
        // Send leave message
        if (!session_id_.empty()) {
            leave_session();
        }
        
        // Close WebSocket connection
        websocketpp::lib::error_code ec;
        ws_client_.close(ws_connection_, websocketpp::close::status::normal, "Client disconnect", ec);
        
        if (ec) {
            log_error("Disconnect error: " + ec.message());
        }
        
        // Stop WebSocket client
        ws_client_.stop();
        
        // Wait for thread to finish
        if (ws_thread_.joinable()) {
            ws_thread_.join();
        }
        
        connected_ = false;
        connected_peers_.clear();
        
        log_info("Disconnected from P2P coordinator");
    } catch (const std::exception& e) {
        log_error("Disconnect exception: " + std::string(e.what()));
    }
}

bool P2PNetwork::is_connected() const {
    return connected_;
}

void P2PNetwork::join_session(const std::string& session_id) {
    session_id_ = session_id;
    
    json message = {
        {"type", "join"},
        {"session_id", session_id},
        {"peer_id", peer_id_}
    };
    
    send_message(message);
    log_info("Joining session: " + session_id);
}

void P2PNetwork::leave_session() {
    if (session_id_.empty()) return;
    
    json message = {
        {"type", "leave"},
        {"session_id", session_id_},
        {"peer_id", peer_id_}
    };
    
    send_message(message);
    log_info("Leaving session: " + session_id_);
    
    session_id_.clear();
    connected_peers_.clear();
}

void P2PNetwork::send_offer(const std::string& to_peer, const json& sdp) {
    json message = {
        {"type", "offer"},
        {"to", to_peer},
        {"from", peer_id_},
        {"sdp", sdp}
    };
    
    send_message(message);
    log_info("Sent offer to peer: " + to_peer);
}

void P2PNetwork::send_answer(const std::string& to_peer, const json& sdp) {
    json message = {
        {"type", "answer"},
        {"to", to_peer},
        {"from", peer_id_},
        {"sdp", sdp}
    };
    
    send_message(message);
    log_info("Sent answer to peer: " + to_peer);
}

void P2PNetwork::send_ice_candidate(const std::string& to_peer, const json& candidate) {
    json message = {
        {"type", "ice-candidate"},
        {"to", to_peer},
        {"from", peer_id_},
        {"candidate", candidate}
    };
    
    send_message(message);
    log_info("Sent ICE candidate to peer: " + to_peer);
}

void P2PNetwork::poll() {
    // WebSocket events are handled in separate thread
    // This method can be used for additional processing if needed
}

// Private methods

void P2PNetwork::on_open(connection_hdl hdl) {
    connected_ = true;
    log_info("WebSocket connection opened");
    
    // Auto-join session if session_id was provided
    if (!session_id_.empty()) {
        join_session(session_id_);
    }
}

void P2PNetwork::on_close(connection_hdl hdl) {
    connected_ = false;
    connected_peers_.clear();
    log_info("WebSocket connection closed");
}

void P2PNetwork::on_fail(connection_hdl hdl) {
    connected_ = false;
    log_error("WebSocket connection failed");

    if (error_callback_) {
        error_callback_("WebSocket connection failed");
    }
}

void P2PNetwork::on_message(connection_hdl hdl, MessagePtr msg) {
    try {
        std::string payload = msg->get_payload();
        json message = json::parse(payload);

        std::string type = message["type"];

        if (type == "session-joined") {
            handle_session_joined(message);
        } else if (type == "peer-joined") {
            handle_peer_joined(message);
        } else if (type == "peer-left") {
            handle_peer_left(message);
        } else if (type == "offer") {
            handle_offer(message);
        } else if (type == "answer") {
            handle_answer(message);
        } else if (type == "ice-candidate") {
            handle_ice_candidate(message);
        } else if (type == "error") {
            handle_error(message);
        } else {
            log_error("Unknown message type: " + type);
        }
    } catch (const std::exception& e) {
        log_error("Message handling exception: " + std::string(e.what()));
    }
}

void P2PNetwork::handle_session_joined(const json& message) {
    connected_peers_.clear();

    if (message.contains("peers") && message["peers"].is_array()) {
        for (const auto& peer : message["peers"]) {
            std::string peer_id = peer.get<std::string>();
            if (peer_id != peer_id_) {
                connected_peers_.push_back(peer_id);
            }
        }
    }

    log_info("Joined session with " + std::to_string(connected_peers_.size()) + " peers");

    if (session_joined_callback_) {
        session_joined_callback_(connected_peers_);
    }
}

void P2PNetwork::handle_peer_joined(const json& message) {
    std::string peer_id = message["peer_id"];

    if (peer_id != peer_id_) {
        connected_peers_.push_back(peer_id);
        log_info("Peer joined: " + peer_id);

        if (peer_joined_callback_) {
            peer_joined_callback_(peer_id);
        }
    }
}

void P2PNetwork::handle_peer_left(const json& message) {
    std::string peer_id = message["peer_id"];

    auto it = std::find(connected_peers_.begin(), connected_peers_.end(), peer_id);
    if (it != connected_peers_.end()) {
        connected_peers_.erase(it);
        log_info("Peer left: " + peer_id);

        if (peer_left_callback_) {
            peer_left_callback_(peer_id);
        }
    }
}

void P2PNetwork::handle_offer(const json& message) {
    std::string from_peer = message["from"];
    json sdp = message["sdp"];

    log_info("Received offer from peer: " + from_peer);

    if (offer_received_callback_) {
        offer_received_callback_(from_peer, sdp);
    }
}

void P2PNetwork::handle_answer(const json& message) {
    std::string from_peer = message["from"];
    json sdp = message["sdp"];

    log_info("Received answer from peer: " + from_peer);

    if (answer_received_callback_) {
        answer_received_callback_(from_peer, sdp);
    }
}

void P2PNetwork::handle_ice_candidate(const json& message) {
    std::string from_peer = message["from"];
    json candidate = message["candidate"];

    log_info("Received ICE candidate from peer: " + from_peer);

    if (ice_candidate_callback_) {
        ice_candidate_callback_(from_peer, candidate);
    }
}

void P2PNetwork::handle_error(const json& message) {
    std::string error_msg = message.value("message", "Unknown error");
    log_error("Coordinator error: " + error_msg);

    if (error_callback_) {
        error_callback_(error_msg);
    }
}

void P2PNetwork::send_message(const json& message) {
    if (!connected_) {
        log_error("Cannot send message: not connected");
        return;
    }

    try {
        std::string payload = message.dump();
        websocketpp::lib::error_code ec;
        ws_client_.send(ws_connection_, payload, websocketpp::frame::opcode::text, ec);

        if (ec) {
            log_error("Send message error: " + ec.message());
        }
    } catch (const std::exception& e) {
        log_error("Send message exception: " + std::string(e.what()));
    }
}

std::string P2PNetwork::build_connection_url(const std::string& base_url,
                                             const std::string& peer_id,
                                                const std::string& session_id) {
    std::ostringstream url;
    url << base_url << "?peer_id=" << peer_id;

    if (!session_id.empty()) {
        url << "&session_id=" << session_id;
    }

    return url.str();
}

void P2PNetwork::log_info(const std::string& message) {
    std::cout << "[P2P] INFO: " << message << std::endl;
}

void P2PNetwork::log_error(const std::string& message) {
    std::cerr << "[P2P] ERROR: " << message << std::endl;
}

