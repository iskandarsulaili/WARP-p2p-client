#include "../../include/PacketRouter.h"
#include "../../include/Logger.h"
#include "../../include/ConfigManager.h"
#include "../../include/WebRTCManager.h"
#include "../../include/PacketSerializer.h"

namespace P2P {

struct PacketRouter::Impl {
    bool p2p_enabled = false;
    std::string current_zone;
    RouteDecision default_route = RouteDecision::SERVER;
    std::shared_ptr<WebRTCManager> webrtc_manager;
    ServerRouteCallback server_callback;
};

PacketRouter::PacketRouter() : impl_(std::make_unique<Impl>()) {
    LOG_DEBUG("PacketRouter created");
}

PacketRouter::~PacketRouter() {
    LOG_DEBUG("PacketRouter destroyed");
}

bool PacketRouter::Initialize(bool p2p_enabled,
                             std::shared_ptr<WebRTCManager> webrtc_manager,
                             ServerRouteCallback server_callback) {
    if (!webrtc_manager) {
        LOG_ERROR("WebRTCManager is null");
        return false;
    }

    if (!server_callback) {
        LOG_ERROR("Server callback is null");
        return false;
    }

    impl_->p2p_enabled = p2p_enabled;
    impl_->webrtc_manager = webrtc_manager;
    impl_->server_callback = server_callback;

    LOG_INFO("PacketRouter initialized (P2P " + std::string(p2p_enabled ? "enabled" : "disabled") + ")");
    return true;
}

void PacketRouter::Shutdown() {
    impl_->p2p_enabled = false;
}

RouteDecision PacketRouter::DecideRoute(const Packet& packet) {
    // If P2P is disabled globally, always route to server
    if (!impl_->p2p_enabled) {
        return RouteDecision::SERVER;
    }
    
    // Check if current zone supports P2P
    auto& config = ConfigManager::GetInstance();
    if (!config.IsZoneP2PEnabled(impl_->current_zone)) {
        return RouteDecision::SERVER;
    }
    
    // Route based on packet type
    // Movement, chat, and item pickup can go P2P
    // Everything else goes to server for validation
    switch (packet.type) {
        case 0x0089: // Movement
        case 0x008C: // Chat
        case 0x009F: // Item pickup
            return RouteDecision::P2P;
        default:
            return RouteDecision::SERVER;
    }
}

bool PacketRouter::RoutePacket(const Packet& packet, RouteDecision decision) {
    if (decision == RouteDecision::SERVER) {
        return RouteToServer(packet);
    } else {
        return RouteToP2P(packet);
    }
}

bool PacketRouter::RouteToServer(const Packet& packet) {
    LOG_DEBUG("Routing packet to server (type: 0x" + std::to_string(packet.type) + ")");

    if (!impl_->server_callback) {
        LOG_ERROR("Server callback not set");
        return false;
    }

    // Serialize packet
    std::vector<uint8_t> serialized;
    if (!PacketSerializer::Serialize(packet, serialized)) {
        LOG_ERROR("Failed to serialize packet for server routing");
        return false;
    }

    // Route to server via callback
    bool success = impl_->server_callback(serialized.data(), serialized.size());

    if (!success) {
        LOG_ERROR("Failed to route packet to server");
    }

    return success;
}

bool PacketRouter::RouteToP2P(const Packet& packet) {
    LOG_DEBUG("Routing packet to P2P (type: 0x" + std::to_string(packet.type) + ")");

    if (!impl_->webrtc_manager) {
        LOG_ERROR("WebRTCManager not set, falling back to server routing");
        return RouteToServer(packet);
    }

    // Serialize packet
    std::vector<uint8_t> serialized;
    if (!PacketSerializer::Serialize(packet, serialized)) {
        LOG_ERROR("Failed to serialize packet for P2P routing, falling back to server");
        return RouteToServer(packet);
    }

    // Send to all connected peers via WebRTC data channels
    bool success = impl_->webrtc_manager->BroadcastData(serialized.data(), serialized.size());

    if (!success) {
        LOG_WARN("Failed to route packet to P2P peers, falling back to server routing");
        // Fallback to server routing if P2P fails
        return RouteToServer(packet);
    }

    LOG_DEBUG("Packet successfully routed to P2P peers");
    return true;
}

void PacketRouter::SetCurrentZone(const std::string& zone) {
    impl_->current_zone = zone;
    LOG_INFO("Zone changed to: " + zone);
}

std::string PacketRouter::GetCurrentZone() const {
    return impl_->current_zone;
}

void PacketRouter::EnableP2P(bool enabled) {
    impl_->p2p_enabled = enabled;
    LOG_INFO("P2P " + std::string(enabled ? "enabled" : "disabled"));
}

bool PacketRouter::IsP2PEnabled() const {
    return impl_->p2p_enabled;
}

} // namespace P2P
