#pragma once

#include <memory>
#include <string>
#include <vector>

namespace P2P {

// Forward declaration
class WebRTCPeerConnection;

/**
 * WebRTCManager - Manages multiple WebRTC peer connections
 */
class WebRTCManager {
public:
    WebRTCManager();
    ~WebRTCManager();

    // AOI/mesh: Set local player position
    void SetLocalPosition(float x, float y, float z);

    // AOI/mesh: Refresh mesh (prune, score, AOI)
    void RefreshMesh();

    // AOI/mesh: Set AOI radius
    void SetAOIRadius(float radius);

    // AOI/mesh: Get AOI radius
    float GetAOIRadius() const;

    // Disable copy and move
    WebRTCManager(const WebRTCManager&) = delete;
    WebRTCManager& operator=(const WebRTCManager&) = delete;
    WebRTCManager(WebRTCManager&&) = delete;
    WebRTCManager& operator=(WebRTCManager&&) = delete;

    /**
     * Initialize the WebRTC manager
     * @param stun STUN server URLs
     * @param turn TURN server URLs
     * @param turn_username TURN server username
     * @param turn_credential TURN server credential
     * @param max Maximum number of peer connections
     * @return true if initialization succeeded
     */
    bool Initialize(const std::vector<std::string>& stun, const std::vector<std::string>& turn,
                   const std::string& turn_username, const std::string& turn_credential, int max);

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
     * Check if any peer is connected
     * @return true if at least one peer is connected
     */
    bool IsConnected() const;

    /**
     * Send data to all connected peers
     * @极 data The data to send
     * @param size The size of the data
     * @return true if data was sent successfully
     */
    bool SendData(const void* data, size_t size);

    /**
     * Process a WebRTC offer from a peer
     * @param offer The SDP offer string
     */
    void ProcessOffer(const std::string& offer);

    /**
     * Add an ICE candidate from a peer
     * @param candidate The ICE candidate string
     */
    void AddIceCandidate(const std::string& candidate);

private:
    // Pimpl idiom for implementation details
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace P2P
