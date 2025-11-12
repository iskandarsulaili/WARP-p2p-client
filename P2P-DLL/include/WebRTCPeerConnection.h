#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace P2P {

/**
 * WebRTCPeerConnection - Manages a WebRTC peer-to-peer connection
 */
class WebRTCPeerConnection {
public:
    // Callback types
    using OnDataCallback = std::function<void(const uint8_t*, size_t)>;
    using OnStateChangeCallback = std::function<void(bool)>;
    using OnIceCandidateCallback = std::function<void(const std::string&)>;

    explicit WebRTCPeerConnection(const std::string& peer_id);
    ~WebRTCPeerConnection();

    // Disable copy and move
    WebRTCPeerConnection(const WebRTCPeerConnection&) = delete;
    WebRTCPeerConnection& operator=(const WebRTCPeerConnection&) = delete;
    WebRTCPeerConnection(WebRTCPeerConnection&&) = delete;
    WebRTCPeerConnection& operator=(WebRTCPeerConnection&&) = delete;

    /**
     * Initialize the peer connection
     * @param stun STUN server URLs
     * @param turn TURN server URLs
     * @return true if initialization succeeded
     */
    bool Initialize(const std::vector<std::string>& stun, const std::vector<std::string>& turn,
                   const std::string& turn_username, const std::string& turn_credential);

    /**
     * Close the peer connection
     */
    void Close();

    /**
     * Create an SDP offer
     * @param sdp_out Output SDP string
     * @return true if offer creation succeeded
     */
    bool CreateOffer(std::string& sdp_out);

    /**
     * Create an SDP answer
     * @param sdp_out Output SDP string
     * @return true if answer creation succeeded
     */
    bool CreateAnswer(std::string& sdp_out);

    /**
     * Set remote SDP description
     * @param sdp The remote SDP
     * @return true if setting succeeded
     */
    bool SetRemoteDescription(const std::string& sdp);

    /**
     * Add ICE candidate
     * @param candidate The ICE candidate string
     * @return true if adding succeeded
     */
    bool AddIceCandidate(const std::string& candidate);

    /**
     * Send data over the data channel
     * @param data Data buffer
     * @param size Data size
     * @return true if send succeeded
     */
    bool SendData(const uint8_t* data, size_t size);

    /**
     * Check if connection is established
     * @return true if connected
     */
    bool IsConnected() const;

    /**
     * Get the peer ID
     * @return The peer ID
     */
    std::string GetPeerId() const;

    /**
     * Set callback for received data
     * @param callback The callback function
     */
    void SetOnDataCallback(OnDataCallback callback);

    /**
     * Set callback for connection state changes
     * @param callback The callback function
     */
    void SetOnStateChangeCallback(OnStateChangeCallback callback);

    /**
     * Set callback for ICE candidates
     * @param callback The callback function
     */
    void SetOnIceCandidateCallback(OnIceCandidateCallback callback);

private:
    // Pimpl idiom for implementation details
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace P2P
