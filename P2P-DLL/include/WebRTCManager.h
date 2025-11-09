#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace P2P {

// Forward declaration
class WebRTCPeerConnection;

// Callback types for signaling
using OnOfferCallback = std::function<void(const std::string& peer_id, const std::string& sdp)>;
using OnAnswerCallback = std::function<void(const std::string& peer_id, const std::string& sdp)>;
using OnIceCandidateCallback = std::function<void(const std::string& peer_id, const std::string& candidate)>;

/**
 * WebRTCManager - Manages multiple WebRTC peer connections
 */
class WebRTCManager {
public:
    WebRTCManager();
    ~WebRTCManager();

    // Disable copy and move
    WebRTCManager(const WebRTCManager&) = delete;
    WebRTCManager& operator=(const WebRTCManager&) = delete;
    WebRTCManager(WebRTCManager&&) = delete;
    WebRTCManager& operator=(WebRTCManager&&) = delete;

    /**
     * Initialize the WebRTC manager
     * @param stun STUN server URLs
     * @param turn TURN server URLs
     * @param max Maximum number of peer connections
     * @return true if initialization succeeded
     */
    bool Initialize(const std::vector<std::string>& stun, const std::vector<std::string>& turn, int max);

    /**
     * Shutdown the WebRTC manager
     */
    void Shutdown();

    /**
     * Create a new peer connection
     * @param peer_id The peer ID
     * @return Shared pointer to the peer connection
     */
    std::shared_ptr<WebRTCPeerConnection> CreatePeerConnection(const std::string& peer_id);

    /**
     * Remove a peer connection
     * @param peer_id The peer ID to remove
     */
    void RemovePeerConnection(const std::string& peer_id);

    /**
     * Get an existing peer connection
     * @param peer_id The peer ID
     * @return Shared pointer to the peer connection, or nullptr if not found
     */
    std::shared_ptr<WebRTCPeerConnection> GetPeerConnection(const std::string& peer_id);

    /**
     * Get list of connected peer IDs
     * @return Vector of connected peer IDs
     */
    std::vector<std::string> GetConnectedPeers() const;

    /**
     * Get the number of peer connections
     * @return Number of peers
     */
    int GetPeerCount() const;

    /**
     * Broadcast data to all connected peers
     * @param data Data to broadcast
     * @param length Length of data
     * @return true if broadcast succeeded to at least one peer
     */
    bool BroadcastData(const uint8_t* data, size_t length);

    /**
     * Send data to a specific peer
     * @param peer_id Target peer ID
     * @param data Data to send
     * @param length Length of data
     * @return true if send succeeded
     */
    bool SendDataToPeer(const std::string& peer_id, const uint8_t* data, size_t length);

    /**
     * Create WebRTC offer for a peer
     * @param peer_id Target peer ID
     * @return true if offer creation initiated
     */
    bool CreateOffer(const std::string& peer_id);

    /**
     * Handle WebRTC offer from a peer
     * @param peer_id Source peer ID
     * @param sdp SDP offer string
     * @return true if offer handled successfully
     */
    bool HandleOffer(const std::string& peer_id, const std::string& sdp);

    /**
     * Handle WebRTC answer from a peer
     * @param peer_id Source peer ID
     * @param sdp SDP answer string
     * @return true if answer handled successfully
     */
    bool HandleAnswer(const std::string& peer_id, const std::string& sdp);

    /**
     * Add ICE candidate for a peer
     * @param peer_id Target peer ID
     * @param candidate ICE candidate string
     * @param sdp_mid SDP media ID
     * @param sdp_mline_index SDP m-line index
     * @return true if candidate added successfully
     */
    bool AddIceCandidate(const std::string& peer_id, const std::string& candidate,
                        const std::string& sdp_mid, int sdp_mline_index);

    /**
     * Close connection to a specific peer
     * @param peer_id Peer ID to disconnect
     */
    void CloseConnection(const std::string& peer_id);

    /**
     * Close all peer connections
     */
    void CloseAllConnections();

    /**
     * Set callback for when an offer is created
     * @param callback Callback function
     */
    void SetOnOfferCallback(OnOfferCallback callback);

    /**
     * Set callback for when an answer is created
     * @param callback Callback function
     */
    void SetOnAnswerCallback(OnAnswerCallback callback);

    /**
     * Set callback for when an ICE candidate is generated
     * @param callback Callback function
     */
    void SetOnIceCandidateCallback(OnIceCandidateCallback callback);

private:
    // Pimpl idiom for implementation details
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace P2P
