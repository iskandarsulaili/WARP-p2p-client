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
     * @return true if initialized successfully, false otherwise
     */
    bool Initialize();

    /**
     * Shutdown network manager
     */
    void Shutdown();

    /**
     * Start P2P networking
     * 
     * @param player_id Player identifier
     * @param user_id User identifier
     * @return true if started successfully, false otherwise
     */
    bool Start(const std::string& player_id, const std::string& user_id);

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

private:
    NetworkManager() = default;
    ~NetworkManager() = default;
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace P2P

