#pragma once

#include "Types.h"
#include <memory>
#include <string>

namespace P2P {

// Forward declarations
class SignalingClient;
class WebRTCManager;
class PacketRouter;
class SecurityManager;

/**
 * Network Manager
 * 
 * Main coordinator for P2P networking.
 * Manages lifecycle of all networking components.
 */
class NetworkManager {
public:
    /**
     * Get singleton instance
     */
    static NetworkManager& GetInstance();

    /**
     * Initialize network manager
     *
     * @param peer_id Peer identifier
     * @return true if initialized successfully, false otherwise
     */
    bool Initialize(const std::string& peer_id);

    /**
     * Shutdown network manager
     */
    void Shutdown();

    /**
     * Start P2P networking
     *
     * @return true if started successfully, false otherwise
     */
    bool Start();

    /**
     * Stop P2P networking
     */
    void Stop();

    /**
     * Check if P2P is active
     */
    bool IsActive() const;

    /**
     * Handle zone change
     * 
     * @param zone_id New zone identifier
     */
    void OnZoneChange(const std::string& zone_id);

    /**
     * Send packet
     *
     * @param packet Packet to send
     * @return true if sent successfully, false otherwise
     */
    bool SendPacket(const Packet& packet);

    /**
     * Get current session ID
     *
     * @return Current session ID or empty string if not in session
     */
    std::string GetCurrentSessionId() const;

    /**
     * Get current zone ID
     *
     * @return Current zone ID or empty string
     */
    std::string GetCurrentZone() const;

private:
    /**
     * Discover or create a session for the given zone
     *
     * @param zone_id Zone identifier
     * @return Session ID or empty string on failure
     */
    std::string DiscoverSession(const std::string& zone_id);

    /**
     * Create a new P2P session
     *
     * @param zone_id Zone identifier
     * @return Session ID or empty string on failure
     */
    std::string CreateSession(const std::string& zone_id);

    /**
     * Join a P2P session
     *
     * @param session_id Session identifier
     * @return true if joined successfully
     */
    bool JoinSession(const std::string& session_id);

    /**
     * Leave current P2P session
     */
    void LeaveSession();

    /**
     * Handle signaling messages
     *
     * @param message JSON message from signaling server
     */
    void HandleSignalingMessage(const std::string& message);
    NetworkManager();
    ~NetworkManager();
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace P2P

