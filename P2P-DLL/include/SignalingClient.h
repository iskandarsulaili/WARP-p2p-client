#pragma once

#include "Types.h"
#include <memory>
#include <string>
#include <functional>

namespace P2P {

/**
 * Signaling Client
 * 
 * WebSocket client for signaling with coordinator service.
 * Handles WebRTC offer/answer/ICE candidate exchange.
 */
class SignalingClient {
public:
    using OnMessageCallback = std::function<void(const std::string&)>;
    using OnConnectedCallback = std::function<void()>;
    using OnDisconnectedCallback = std::function<void()>;

    SignalingClient();
    ~SignalingClient();

    /**
     * Connect to signaling server
     * 
     * @param url WebSocket URL
     * @param peer_id Peer identifier
     * @param session_id Session identifier
     * @return true if connected successfully, false otherwise
     */
    bool Connect(const std::string& url, const std::string& peer_id, const std::string& session_id);

    /**
     * Disconnect from signaling server
     */
    void Disconnect();

    /**
     * Check if connected
     */
    bool IsConnected() const;

    /**
     * Send message
     * 
     * @param message JSON message to send
     * @return true if sent successfully, false otherwise
     */
    bool SendMessage(const std::string& message);

    /**
     * Set message callback
     */
    void SetOnMessageCallback(OnMessageCallback callback);

    /**
     * Set connected callback
     */
    void SetOnConnectedCallback(OnConnectedCallback callback);

    /**
     * Set disconnected callback
     */
    void SetOnDisconnectedCallback(OnDisconnectedCallback callback);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace P2P

