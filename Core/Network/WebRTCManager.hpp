#ifndef WEBRTC_MANAGER_HPP
#define WEBRTC_MANAGER_HPP

#include "P2PNetwork.hpp"
#include "WebRTCPeerConnection.hpp"
#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include <string>

/**
 * @brief WebRTC Manager - Integrates WebRTC with P2P signaling
 * 
 * Manages multiple WebRTC peer connections and coordinates with the P2P
 * signaling network for session management and peer discovery.
 * 
 * Architecture:
 *   P2PNetwork (WebSocket) -> Signaling (offer/answer/ICE)
 *   WebRTCManager -> Manages peer connections
 *   WebRTCPeerConnection -> Individual P2P data channels
 * 
 * Thread Safety: All public methods are thread-safe
 * 
 * Usage:
 *   WebRTCManager manager;
 *   manager.initialize(p2p_network, stun_servers);
 *   manager.connect_to_peer("peer-123");
 *   manager.send_to_peer("peer-123", data);
 */
class WebRTCManager {
public:
    /**
     * @brief Bandwidth management configuration
     */
    struct BandwidthConfig {
        uint32_t max_bitrate_kbps;          // Maximum bitrate in kbps
        uint32_t min_bitrate_kbps;          // Minimum bitrate in kbps
        uint32_t start_bitrate_kbps;        // Starting bitrate in kbps
        uint32_t max_packet_size_bytes;     // Maximum packet size
        bool enable_congestion_control;     // Enable congestion control
    };
    
    /**
     * @brief Peer connection info
     */
    struct PeerInfo {
        std::string peer_id;
        WebRTCPeerConnection::State state;
        WebRTCPeerConnection::DataChannelState data_channel_state;
        uint64_t bytes_sent;
        uint64_t bytes_received;
        uint32_t latency_ms;
    };
    
    // Callback type definitions
    using PeerConnectedCallback = std::function<void(const std::string& peer_id)>;
    using PeerDisconnectedCallback = std::function<void(const std::string& peer_id)>;
    using MessageReceivedCallback = std::function<void(const std::string& peer_id, const std::vector<uint8_t>& data)>;
    using ErrorCallback = std::function<void(const std::string& error)>;
    
    /**
     * @brief Constructor
     */
    WebRTCManager();
    
    /**
     * @brief Destructor
     */
    ~WebRTCManager();
    
    // Disable copy
    WebRTCManager(const WebRTCManager&) = delete;
    WebRTCManager& operator=(const WebRTCManager&) = delete;
    
    /**
     * @brief Initialize WebRTC manager
     * @param p2p_network P2P signaling network
     * @param stun_servers List of STUN server URLs
     * @param turn_servers List of TURN server URLs (optional)
     * @param bandwidth_config Bandwidth configuration (optional)
     * @return true if initialization successful
     */
    bool initialize(P2PNetwork* p2p_network,
                   const std::vector<std::string>& stun_servers,
                   const std::vector<std::string>& turn_servers = {},
                   const BandwidthConfig& bandwidth_config = {});
    
    /**
     * @brief Shutdown WebRTC manager
     */
    void shutdown();
    
    /**
     * @brief Connect to a peer
     * @param peer_id Peer ID to connect to
     * @return true if connection initiated
     */
    bool connect_to_peer(const std::string& peer_id);
    
    /**
     * @brief Disconnect from a peer
     * @param peer_id Peer ID to disconnect from
     */
    void disconnect_from_peer(const std::string& peer_id);
    
    /**
     * @brief Send data to a peer
     * @param peer_id Peer ID to send to
     * @param data Binary data to send
     * @return true if data sent successfully
     */
    bool send_to_peer(const std::string& peer_id, const std::vector<uint8_t>& data);
    
    /**
     * @brief Send message to a peer
     * @param peer_id Peer ID to send to
     * @param message String message to send
     * @return true if message sent successfully
     */
    bool send_message_to_peer(const std::string& peer_id, const std::string& message);
    
    /**
     * @brief Broadcast data to all connected peers
     * @param data Binary data to broadcast
     * @return Number of peers data was sent to
     */
    size_t broadcast(const std::vector<uint8_t>& data);
    
    /**
     * @brief Get list of connected peers
     */
    std::vector<std::string> get_connected_peers() const;
    
    /**
     * @brief Get peer information
     * @param peer_id Peer ID
     * @return Peer info or nullptr if not found
     */
    std::unique_ptr<PeerInfo> get_peer_info(const std::string& peer_id) const;
    
    /**
     * @brief Get all peer information
     */
    std::vector<PeerInfo> get_all_peer_info() const;
    
    /**
     * @brief Check if connected to peer
     */
    bool is_connected_to_peer(const std::string& peer_id) const;
    
    /**
     * @brief Get number of connected peers
     */
    size_t get_peer_count() const;
    
    /**
     * @brief Update - call from main loop
     */
    void update();
    
    // Event callbacks
    void on_peer_connected(PeerConnectedCallback callback) { peer_connected_callback_ = callback; }
    void on_peer_disconnected(PeerDisconnectedCallback callback) { peer_disconnected_callback_ = callback; }
    void on_message_received(MessageReceivedCallback callback) { message_received_callback_ = callback; }
    void on_error(ErrorCallback callback) { error_callback_ = callback; }

private:
    P2PNetwork* p2p_network_;
    std::vector<std::string> stun_servers_;
    std::vector<std::string> turn_servers_;
    BandwidthConfig bandwidth_config_;
    
    // Peer connections
    std::map<std::string, std::unique_ptr<WebRTCPeerConnection>> peer_connections_;
    mutable std::mutex peers_mutex_;
    
    // Callbacks
    PeerConnectedCallback peer_connected_callback_;
    PeerDisconnectedCallback peer_disconnected_callback_;
    MessageReceivedCallback message_received_callback_;
    ErrorCallback error_callback_;
    
    // Internal methods
    void setup_p2p_callbacks();
    void handle_offer_received(const std::string& from_peer, const json& sdp);
    void handle_answer_received(const std::string& from_peer, const json& sdp);
    void handle_ice_candidate_received(const std::string& from_peer, const json& candidate);
    void handle_peer_joined(const std::string& peer_id);
    void handle_peer_left(const std::string& peer_id);
    
    WebRTCPeerConnection* get_or_create_peer_connection(const std::string& peer_id);
    void remove_peer_connection(const std::string& peer_id);
    
    void log_info(const std::string& message);
    void log_error(const std::string& message);
};

#endif // WEBRTC_MANAGER_HPP

