#pragma once

#include "Types.h"
#include <memory>
#include <string>
#include <functional>

namespace P2P {

// Forward declarations
class WebRTCManager;

/**
 * Callback for routing packets to the original server
 * @param data Packet data
 * @param length Packet length
 * @return true if packet was sent successfully
 */
using ServerRouteCallback = std::function<bool(const uint8_t* data, size_t length)>;

/**
 * PacketRouter - Routes packets between P2P and server based on configuration
 */
class PacketRouter {
public:
    PacketRouter();
    ~PacketRouter();

    // Disable copy and move
    PacketRouter(const PacketRouter&) = delete;
    PacketRouter& operator=(const PacketRouter&) = delete;
    PacketRouter(PacketRouter&&) = delete;
    PacketRouter& operator=(PacketRouter&&) = delete;

    /**
     * Initialize the packet router
     * @param p2p_enabled Whether P2P routing is enabled
     * @param webrtc_manager WebRTC manager for P2P routing
     * @param server_callback Callback for routing to server
     * @return true if initialization succeeded
     */
    bool Initialize(bool p2p_enabled,
                   std::shared_ptr<WebRTCManager> webrtc_manager,
                   ServerRouteCallback server_callback);

    /**
     * Shutdown the packet router
     */
    void Shutdown();

    /**
     * Decide how to route a packet
     * @param packet The packet to route
     * @return The routing decision
     */
    RouteDecision DecideRoute(const Packet& packet);

    /**
     * Route a packet based on the decision
     * @param packet The packet to route
     * @param decision The routing decision
     * @return true if routing succeeded
     */
    bool RoutePacket(const Packet& packet, RouteDecision decision);

    /**
     * Set the current zone
     * @param zone The zone ID
     */
    void SetCurrentZone(const std::string& zone);

    /**
     * Get the current zone
     * @return The current zone ID
     */
    std::string GetCurrentZone() const;

    /**
     * Enable or disable P2P routing
     * @param enabled Whether P2P should be enabled
     */
    void EnableP2P(bool enabled);

    /**
     * Check if P2P routing is enabled
     * @return true if P2P is enabled
     */
    bool IsP2PEnabled() const;

private:
    /**
     * Route packet to server
     */
    bool RouteToServer(const Packet& packet);

    /**
     * Route packet to P2P peers
     */
    bool RouteToP2P(const Packet& packet);

    // Pimpl idiom for implementation details
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace P2P
