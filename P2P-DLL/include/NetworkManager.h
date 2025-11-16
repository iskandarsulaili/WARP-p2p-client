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
class BandwidthManager;
class CompressionManager;

// Forward declaration for QUIC transport
class QuicTransport;
class ITransport;
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
     * Get BandwidthManager instance
     *
     * @return Reference to BandwidthManager
     */
    BandwidthManager& GetBandwidthManager();

    /**
     * Get CompressionManager instance
     *
    /**
     * Select transport protocol (QUIC or WebRTC) based on configuration and server capabilities.
     * @param prefer_quic If true, prefer QUIC over WebRTC.
     */
    void SelectTransport(bool prefer_quic);

    /**
     * Get current transport (ITransport).
     */
    ITransport* GetTransport() const;
     * @return Reference to CompressionManager
     */
    CompressionManager& GetCompressionManager();

private:
    NetworkManager();
    ~NetworkManager();
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    /**
     * Handle signaling messages from coordinator
     */
    void HandleSignalingMessage(const std::string& message);

    /**
     * Send session request to coordinator
     */
    void SendSessionRequest();

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace P2P

