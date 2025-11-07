#ifndef WEBRTC_PEER_CONNECTION_HPP
#define WEBRTC_PEER_CONNECTION_HPP

#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <mutex>
#include <nlohmann/json.hpp>

// Forward declarations for WebRTC native API
// In production, include actual WebRTC headers: #include <api/peer_connection_interface.h>
namespace webrtc {
    class PeerConnectionInterface;
    class DataChannelInterface;
    class IceCandidateInterface;
    class SessionDescriptionInterface;
    class PeerConnectionFactoryInterface;
}

using json = nlohmann::json;

/**
 * @brief WebRTC Peer Connection Manager
 * 
 * Manages WebRTC peer-to-peer connections for game data transmission.
 * Handles peer connection lifecycle, data channels, and ICE negotiation.
 * 
 * Thread Safety: All public methods are thread-safe
 * 
 * Usage:
 *   WebRTCPeerConnection peer("peer-123");
 *   peer.on_data_channel_open([]() { ... });
 *   peer.on_message_received([](const std::vector<uint8_t>& data) { ... });
 *   peer.create_offer();
 */
class WebRTCPeerConnection {
public:
    /**
     * @brief Connection state enumeration
     */
    enum class State {
        NEW,              // Initial state
        CONNECTING,       // ICE negotiation in progress
        CONNECTED,        // Connection established
        DISCONNECTED,     // Connection lost
        FAILED,           // Connection failed
        CLOSED            // Connection closed
    };
    
    /**
     * @brief Data channel state enumeration
     */
    enum class DataChannelState {
        CONNECTING,
        OPEN,
        CLOSING,
        CLOSED
    };
    
    // Callback type definitions
    using StateCallback = std::function<void(State)>;
    using DataChannelCallback = std::function<void()>;
    using MessageCallback = std::function<void(const std::vector<uint8_t>&)>;
    using ICECandidateCallback = std::function<void(const json&)>;
    using SDPCallback = std::function<void(const json&)>;
    using ErrorCallback = std::function<void(const std::string&)>;
    
    /**
     * @brief Constructor
     * @param peer_id Unique identifier for this peer
     */
    explicit WebRTCPeerConnection(const std::string& peer_id);
    
    /**
     * @brief Destructor - cleans up WebRTC resources
     */
    ~WebRTCPeerConnection();
    
    // Disable copy
    WebRTCPeerConnection(const WebRTCPeerConnection&) = delete;
    WebRTCPeerConnection& operator=(const WebRTCPeerConnection&) = delete;
    
    /**
     * @brief Initialize WebRTC peer connection
     * @param stun_servers List of STUN server URLs
     * @param turn_servers List of TURN server URLs (optional)
     * @return true if initialization successful
     */
    bool initialize(const std::vector<std::string>& stun_servers,
                   const std::vector<std::string>& turn_servers = {});
    
    /**
     * @brief Create WebRTC offer (caller side)
     * @return true if offer creation initiated
     */
    bool create_offer();
    
    /**
     * @brief Create WebRTC answer (callee side)
     * @return true if answer creation initiated
     */
    bool create_answer();
    
    /**
     * @brief Set remote SDP (offer or answer)
     * @param sdp SDP in JSON format
     * @return true if SDP set successfully
     */
    bool set_remote_sdp(const json& sdp);
    
    /**
     * @brief Add ICE candidate
     * @param candidate ICE candidate in JSON format
     * @return true if candidate added successfully
     */
    bool add_ice_candidate(const json& candidate);
    
    /**
     * @brief Send data through data channel
     * @param data Binary data to send
     * @return true if data sent successfully
     */
    bool send_data(const std::vector<uint8_t>& data);
    
    /**
     * @brief Send string message through data channel
     * @param message String message to send
     * @return true if message sent successfully
     */
    bool send_message(const std::string& message);
    
    /**
     * @brief Close peer connection
     */
    void close();
    
    /**
     * @brief Get current connection state
     */
    State get_state() const;
    
    /**
     * @brief Get data channel state
     */
    DataChannelState get_data_channel_state() const;
    
    /**
     * @brief Get peer ID
     */
    std::string get_peer_id() const { return peer_id_; }
    
    /**
     * @brief Check if connection is established
     */
    bool is_connected() const { return state_ == State::CONNECTED; }
    
    /**
     * @brief Check if data channel is open
     */
    bool is_data_channel_open() const { return data_channel_state_ == DataChannelState::OPEN; }
    
    // Event callbacks
    void on_state_change(StateCallback callback) { state_callback_ = callback; }
    void on_data_channel_open(DataChannelCallback callback) { data_channel_open_callback_ = callback; }
    void on_data_channel_close(DataChannelCallback callback) { data_channel_close_callback_ = callback; }
    void on_message_received(MessageCallback callback) { message_callback_ = callback; }
    void on_ice_candidate(ICECandidateCallback callback) { ice_candidate_callback_ = callback; }
    void on_offer_created(SDPCallback callback) { offer_callback_ = callback; }
    void on_answer_created(SDPCallback callback) { answer_callback_ = callback; }
    void on_error(ErrorCallback callback) { error_callback_ = callback; }
    
    /**
     * @brief Get connection statistics
     */
    struct Statistics {
        uint64_t bytes_sent;
        uint64_t bytes_received;
        uint64_t packets_sent;
        uint64_t packets_received;
        uint32_t current_round_trip_time_ms;
        uint32_t available_outgoing_bitrate;
    };
    
    Statistics get_statistics() const;

private:
    std::string peer_id_;
    State state_;
    DataChannelState data_channel_state_;
    
    // WebRTC objects (using raw pointers for compatibility)
    webrtc::PeerConnectionFactoryInterface* pc_factory_;
    webrtc::PeerConnectionInterface* peer_connection_;
    webrtc::DataChannelInterface* data_channel_;
    
    // Thread safety
    mutable std::mutex mutex_;
    
    // Callbacks
    StateCallback state_callback_;
    DataChannelCallback data_channel_open_callback_;
    DataChannelCallback data_channel_close_callback_;
    MessageCallback message_callback_;
    ICECandidateCallback ice_candidate_callback_;
    SDPCallback offer_callback_;
    SDPCallback answer_callback_;
    ErrorCallback error_callback_;
    
    // Statistics
    mutable Statistics stats_;
    
    // Internal methods
    void set_state(State new_state);
    void set_data_channel_state(DataChannelState new_state);
    void handle_ice_candidate(const webrtc::IceCandidateInterface* candidate);
    void handle_data_channel_state_change();
    void handle_data_channel_message(const uint8_t* data, size_t size);
    void create_data_channel();
    void log_info(const std::string& message);
    void log_error(const std::string& message);
};

#endif // WEBRTC_PEER_CONNECTION_HPP

