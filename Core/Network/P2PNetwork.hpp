#pragma once

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;
using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

/**
 * @brief P2P Network Manager with WebSocket-based signaling
 * 
 * Connects to rathena-AI-world P2P coordinator service for WebRTC signaling.
 * Handles session management, peer discovery, and SDP/ICE candidate exchange.
 */
class P2PNetwork {
public:
    using WebSocketClient = websocketpp::client<websocketpp::config::asio_tls_client>;
    using MessagePtr = WebSocketClient::message_ptr;
    
    // Event callbacks
    using SessionJoinedCallback = std::function<void(const std::vector<std::string>&)>;
    using PeerJoinedCallback = std::function<void(const std::string&)>;
    using PeerLeftCallback = std::function<void(const std::string&)>;
    using OfferReceivedCallback = std::function<void(const std::string&, const json&)>;
    using AnswerReceivedCallback = std::function<void(const std::string&, const json&)>;
    using IceCandidateCallback = std::function<void(const std::string&, const json&)>;
    using ErrorCallback = std::function<void(const std::string&)>;
    
    P2PNetwork();
    ~P2PNetwork();
    
    /**
     * @brief Connect to P2P coordinator WebSocket endpoint
     * @param url WebSocket URL (e.g., "ws://localhost:8001/api/signaling/ws")
     * @param peer_id Unique identifier for this peer
     * @param session_id Optional session ID to join
     * @return true if connection initiated successfully
     */
    bool connect(const std::string& url, const std::string& peer_id, 
                 const std::string& session_id = "");
    
    /**
     * @brief Disconnect from coordinator
     */
    void disconnect();
    
    /**
     * @brief Check if connected to coordinator
     */
    bool is_connected() const;
    
    /**
     * @brief Join a P2P session
     * @param session_id Session identifier
     */
    void join_session(const std::string& session_id);
    
    /**
     * @brief Leave current session
     */
    void leave_session();
    
    /**
     * @brief Send WebRTC offer to peer
     * @param to_peer Target peer ID
     * @param sdp SDP offer data
     */
    void send_offer(const std::string& to_peer, const json& sdp);
    
    /**
     * @brief Send WebRTC answer to peer
     * @param to_peer Target peer ID
     * @param sdp SDP answer data
     */
    void send_answer(const std::string& to_peer, const json& sdp);
    
    /**
     * @brief Send ICE candidate to peer
     * @param to_peer Target peer ID
     * @param candidate ICE candidate data
     */
    void send_ice_candidate(const std::string& to_peer, const json& candidate);
    
    /**
     * @brief Process network events (call regularly from main loop)
     */
    void poll();
    
    // Event handler setters
    void on_session_joined(SessionJoinedCallback callback) { session_joined_callback_ = callback; }
    void on_peer_joined(PeerJoinedCallback callback) { peer_joined_callback_ = callback; }
    void on_peer_left(PeerLeftCallback callback) { peer_left_callback_ = callback; }
    void on_offer_received(OfferReceivedCallback callback) { offer_received_callback_ = callback; }
    void on_answer_received(AnswerReceivedCallback callback) { answer_received_callback_ = callback; }
    void on_ice_candidate(IceCandidateCallback callback) { ice_candidate_callback_ = callback; }
    void on_error(ErrorCallback callback) { error_callback_ = callback; }
    
    // Getters
    const std::string& peer_id() const { return peer_id_; }
    const std::string& session_id() const { return session_id_; }
    const std::vector<std::string>& connected_peers() const { return connected_peers_; }
    
private:
    // WebSocket event handlers
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_fail(connection_hdl hdl);
    void on_message(connection_hdl hdl, MessagePtr msg);
    
    // Message handlers
    void handle_session_joined(const json& message);
    void handle_peer_joined(const json& message);
    void handle_peer_left(const json& message);
    void handle_offer(const json& message);
    void handle_answer(const json& message);
    void handle_ice_candidate(const json& message);
    void handle_error(const json& message);
    
    // Helper methods
    void send_message(const json& message);
    std::string build_connection_url(const std::string& base_url, 
                                     const std::string& peer_id,
                                     const std::string& session_id);
    
    // WebSocket client
    WebSocketClient ws_client_;
    connection_hdl ws_connection_;
    std::thread ws_thread_;
    
    // State
    std::string peer_id_;
    std::string session_id_;
    std::vector<std::string> connected_peers_;
    bool connected_;
    
    // Event callbacks
    SessionJoinedCallback session_joined_callback_;
    PeerJoinedCallback peer_joined_callback_;
    PeerLeftCallback peer_left_callback_;
    OfferReceivedCallback offer_received_callback_;
    AnswerReceivedCallback answer_received_callback_;
    IceCandidateCallback ice_candidate_callback_;
    ErrorCallback error_callback_;
    
    // Logging
    void log_info(const std::string& message);
    void log_error(const std::string& message);
};

