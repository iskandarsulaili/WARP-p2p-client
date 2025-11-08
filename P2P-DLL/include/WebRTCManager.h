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

private:
    // Pimpl idiom for implementation details
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace P2P
