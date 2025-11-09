#include "../../include/NetworkManager.h"
#include "../../include/Logger.h"
#include <windows.h>
#include <cstdint>

// Export function for packet hooking
// Called by NEMO patch to intercept send/recv calls
// Returns: 0 = route to server (call original), 1 = handled by P2P (skip original)
extern "C" __declspec(dllexport) int __stdcall P2P_RoutePacket(
    SOCKET socket,
    char* buffer,
    int length,
    int flags,
    int isSend
) {
    try {
        // Get NetworkManager instance
        auto& network_manager = P2P::NetworkManager::GetInstance();
        
        if (!network_manager.IsRunning()) {
            // P2P not active, route to server
            return 0;
        }
        
        // Convert buffer to packet data
        auto* data = reinterpret_cast<uint8_t*>(buffer);
        
        if (isSend) {
            // Outgoing packet - check if it should be routed to P2P
            if (length < 2) {
                // Invalid packet, route to server
                return 0;
            }
            
            // Extract packet ID (first 2 bytes, little-endian)
            uint16_t packet_id = *reinterpret_cast<uint16_t*>(data);
            
            LOG_DEBUG("Intercepted outgoing packet: 0x" + 
                     std::to_string(packet_id) + 
                     ", length: " + std::to_string(length));
            
            // Route packet through NetworkManager
            // This will determine if it should go to P2P or server
            bool handled = network_manager.SendPacket(data, static_cast<size_t>(length));
            
            // Return 1 if P2P handled it, 0 if it should go to server
            return handled ? 1 : 0;
            
        } else {
            // Incoming packet - P2P packets are injected directly
            // We don't intercept recv(), only send()
            // Recv interception would be used for packet filtering/logging
            return 0;
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("P2P_RoutePacket exception: " + std::string(e.what()));
        // On error, route to server
        return 0;
    }
}

// Export function to inject received P2P packets into the game
// This is called by the P2P system when packets are received from peers
extern "C" __declspec(dllexport) void __stdcall P2P_InjectPacket(
    const uint8_t* data,
    size_t length
) {
    try {
        // This would inject the packet into the game's receive buffer
        // Implementation depends on how the RO client processes packets
        
        // For now, just log it
        if (length >= 2) {
            uint16_t packet_id = *reinterpret_cast<const uint16_t*>(data);
            LOG_DEBUG("Injecting P2P packet: 0x" + 
                     std::to_string(packet_id) + 
                     ", length: " + std::to_string(length));
        }
        
        // TODO: Actual packet injection would require:
        // 1. Finding the game's packet receive buffer
        // 2. Writing the packet data to the buffer
        // 3. Triggering the game's packet processing
        // This is highly client-version specific
        
    } catch (const std::exception& e) {
        LOG_ERROR("P2P_InjectPacket exception: " + std::string(e.what()));
    }
}

// Export function to check if P2P is active
extern "C" __declspec(dllexport) int __stdcall P2P_IsActive() {
    try {
        auto& network_manager = P2P::NetworkManager::GetInstance();
        return network_manager.IsRunning() ? 1 : 0;
    } catch (...) {
        return 0;
    }
}

// Export function to get P2P status information
extern "C" __declspec(dllexport) void __stdcall P2P_GetStatus(
    int* is_running,
    int* peer_count,
    int* session_active
) {
    try {
        auto& network_manager = P2P::NetworkManager::GetInstance();
        
        if (is_running) {
            *is_running = network_manager.IsRunning() ? 1 : 0;
        }
        
        if (peer_count) {
            *peer_count = 0; // TODO: Get actual peer count from WebRTCManager
        }
        
        if (session_active) {
            *session_active = 0; // TODO: Check if in active P2P session
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("P2P_GetStatus exception: " + std::string(e.what()));
        if (is_running) *is_running = 0;
        if (peer_count) *peer_count = 0;
        if (session_active) *session_active = 0;
    }
}

