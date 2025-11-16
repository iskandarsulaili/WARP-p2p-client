#include "../../include/PacketRouter.h"
#include "../../include/Logger.h"
#include "../../include/bandwidth/BandwidthManager.h"
#include "../../include/WebRTCManager.h"
#include "../../include/SecurityManager.h"
#include <thread>
#include <chrono>
#include <algorithm>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <detours/detours.h>

namespace P2P {

struct PacketRouter::Impl {
    bool p2p_enabled = false;
    std::string current_zone;
    WebRTCManager* webrtc_manager = nullptr;
    BandwidthManager* bandwidth_manager = nullptr;
    SecurityManager* security_manager = nullptr;
    std::function<bool(const Packet&)> server_send_func;
    
    // Statistics
    uint64_t packets_routed_to_server = 0;
    uint64_t packets_routed_to_p2p = 0;
    uint64_t packets_dropped = 0;
    
    // Configuration
    bool bandwidth_management_enabled = true;
    bool qos_enabled = true;
};

PacketRouter::PacketRouter() : impl_(std::make_unique<Impl>()) {
    LOG_DEBUG("PacketRouter created");
}

PacketRouter::~PacketRouter() {
    Shutdown();
}

bool PacketRouter::Initialize(bool p2p_enabled) {
    impl_->p2p_enabled = p2p_enabled;
    LOG_INFO("PacketRouter initialized with P2P " + std::string(p2p_enabled ? "enabled" : "disabled"));
    return true;
}

void PacketRouter::Shutdown() {
    LOG_DEBUG("PacketRouter shutdown");
}

RouteDecision PacketRouter::DecideRoute(const Packet& packet) {
    if (!impl_->p2p_enabled) {
        return RouteDecision::SERVER;
    }
    
    // Critical packets always use best available route
    if (packet.type <= static_cast<uint16_t>(PacketPriority::CRITICAL)) {
        return RouteDecision::P2P;
    }
    
    // Use P2P for packets that benefit from low latency
    switch (packet.type) {
        case 0x0089: // Movement
        case 0x0090: // Attack
        case 0x0091: // Skill use
        case 0x009F: // Item pickup
            return RouteDecision::P2P;
            
        default:
            // For other packets, use server routing
            return RouteDecision::SERVER;
    }
}


bool PacketRouter::RoutePacket(const Packet& packet, RouteDecision decision) {
    LOG_DEBUG("Routing packet: id=" + std::to_string(packet.packet_id) +
              " type=0x" + std::to_string(packet.type) +
              " length=" + std::to_string(packet.length) +
              " decision=" + std::to_string(static_cast<int>(decision)));
    switch (decision) {
        case RouteDecision::P2P:
            LOG_INFO("Routing to P2P: packet_id=" + std::to_string(packet.packet_id));
            return RouteToP2P(packet);
        case RouteDecision::SERVER:
            LOG_INFO("Routing to server: packet_id=" + std::to_string(packet.packet_id));
            return RouteToServer(packet);
        case RouteDecision::BROADCAST:
            // Broadcast: send to both P2P and server, return true if both succeed
            LOG_INFO("Broadcast routing: packet_id=" + std::to_string(packet.packet_id));
            return RouteToP2P(packet) & RouteToServer(packet);
        case RouteDecision::DROP:
            impl_->packets_dropped++;
            LOG_WARN("Packet dropped: packet_id=" + std::to_string(packet.packet_id));
            return true;
        default:
            LOG_ERROR("Unknown routing decision: packet_id=" + std::to_string(packet.packet_id));
            return false;
    }
}
void PacketRouter::SetCurrentZone(const std::string& zone) {
    impl_->current_zone = zone;
    LOG_DEBUG("Current zone set to: " + zone);
}

std::string PacketRouter::GetCurrentZone() const {
    return impl_->current_zone;
}

void PacketRouter::EnableP2P(bool enabled) {
    impl_->p2p_enabled = enabled;
    LOG_INFO("P2P routing " + std::string(enabled ? "enabled" : "disabled"));
}

bool PacketRouter::IsP2PEnabled() const {
    return impl_->p2p_enabled;
}

void PacketRouter::SetWebRTCManager(WebRTCManager* webrtc_manager) {
    impl_->webrtc_manager = webrtc_manager;
}

void PacketRouter::SetBandwidthManager(BandwidthManager* bandwidth_manager) {
    impl_->bandwidth_manager = bandwidth_manager;
}

bool PacketRouter::RouteToServer(const Packet& packet) {
    if (packet.data.empty() || packet.length == 0) {
        LOG_ERROR("Invalid packet data for server routing");
        return false;
    }
    if (!impl_->server_send_func) {
        LOG_ERROR("No server send function set for PacketRouter");
        return false;
    }
    bool result = impl_->server_send_func(packet);
    if (result) {
        impl_->packets_routed_to_server++;
        LOG_DEBUG("Packet routed to server: type=0x" +
                 std::to_string(packet.type) + ", size=" + std::to_string(packet.length));
    } else {
        LOG_ERROR("Failed to send packet to server: type=0x" + std::to_string(packet.type));
    }
    return result;
}

bool PacketRouter::RouteToP2P(const Packet& packet) {
    if (packet.data.empty() || packet.length == 0) {
        LOG_ERROR("Invalid packet data for P2P routing");
        return false;
    }

    if (!impl_->webrtc_manager || !impl_->webrtc_manager->IsConnected()) {
        LOG_WARN("P2P not connected, falling back to server-only mode");
        impl_->p2p_enabled = false;
        LOG_INFO("Switched to server-only mode due to P2P failure or disconnect");
        return RouteToServer(packet);
    }

    // ED25519 signature (outbound)
    std::vector<uint8_t> signed_packet;
    std::vector<uint8_t> signature(64, 0);
    bool signature_ok = false;
    SecurityManager* sec_mgr = impl_->security_manager;
    if (sec_mgr && sec_mgr->IsSignatureEnabled()) {
        signature_ok = sec_mgr->SignPacketED25519(packet.data.data(), packet.length, signature);
        if (signature_ok) {
            signed_packet.reserve(packet.length + signature.size());
            signed_packet.insert(signed_packet.end(), packet.data.begin(), packet.data.end());
            signed_packet.insert(signed_packet.end(), signature.begin(), signature.end());
            LOG_DEBUG("ED25519 signature appended to outbound P2P packet");
        } else {
            LOG_WARN("Failed to generate ED25519 signature for outbound P2P packet, sending unsigned");
            signed_packet = packet.data;
        }
    } else {
        signed_packet = packet.data;
    }

    // Use WebRTC data channels for P2P routing
    if (impl_->webrtc_manager->SendData(signed_packet.data(), signed_packet.size())) {
        impl_->packets_routed_to_p2p++;

        LOG_DEBUG("Packet routed to P2P: type=0x" +
                 std::to_string(packet.type) + ", size=" + std::to_string(signed_packet.size()));
        return true;
    } else {
        LOG_ERROR("Failed to send packet via P2P, falling back to server");
        return RouteToServer(packet);
    }
}

void PacketRouter::SetSecurityManager(SecurityManager* security_manager) {
    impl_->security_manager = security_manager;
}

void PacketRouter::SetServerSendFunction(std::function<bool(const Packet&)> send_func) {
    impl_->server_send_func = std::move(send_func);
}

} // namespace P2P
