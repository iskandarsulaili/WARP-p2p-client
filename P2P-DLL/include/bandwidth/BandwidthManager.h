#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <chrono>
#include <mutex>
#include <atomic>
#include <string>
#include <functional>
#include <array>
#include "../Types.h"

namespace P2P {

// QoS metrics structure
struct QoSStats {
    double latency_ms = 0.0;          // Round-trip time
    double jitter_ms = 0.0;           // Jitter (latency variation)
    double packet_loss_rate = 0.0;    // Packet loss percentage
    double bandwidth_kbps = 0.0;      // Available bandwidth
    uint64_t packets_sent = 0;        // Total packets sent
    uint64_t packets_received = 0;    // Total packets received
    uint64_t bytes_sent = 0;          // Total bytes sent
    uint64_t bytes_received = 0;      // Total bytes received

    // Reset statistics
    void Reset();
    
    // Convert to string for logging
    std::string ToString() const;
};

// Enhanced Bandwidth configuration (extends existing BandwidthConfig)
struct EnhancedBandwidthConfig : BandwidthConfig {
    uint32_t critical_reserve_kbps = 128;    // Reserved for critical packets
    uint32_t high_priority_kbps = 256;       // For high priority packets
    uint32_t normal_priority_kbps = 512;     // For normal priority packets
    uint32_t low_priority_kbps = 768;        // For low priority packets
    
    uint32_t max_packet_size = 1400;         // Maximum packet size (MTU)
    uint32_t min_packet_size = 64;           // Minimum packet size
    
    // Congestion control parameters
    double congestion_threshold = 0.8;       // 80% bandwidth usage
    double backoff_factor = 0.5;             // Reduce rate by 50% on congestion
    uint32_t backoff_time_ms = 1000;         // Backoff duration
    
    // Validation
    bool IsValid() const;
};

class BandwidthManager {
public:
    BandwidthManager();
    ~BandwidthManager();

    // Initialization and configuration
    bool Initialize(const EnhancedBandwidthConfig& config);
    void Shutdown();
    
    // Bandwidth control
    bool CanSendPacket(PacketPriority priority, uint32_t size);
    void PacketSent(PacketPriority priority, uint32_t size);
    void PacketReceived(uint32_t size);
    
    // Rate limiting
    uint32_t GetAvailableBandwidth(PacketPriority priority) const;
    uint32_t GetCurrentUsage(PacketPriority priority) const;
    
    // QoS monitoring
    const QoSStats& GetQoSStats() const;
    void UpdateLatency(double latency_ms);
    void UpdatePacketLoss(double loss_rate);
    
    // Congestion control
    bool IsCongested() const;
    void HandleCongestion();
    
    // Dynamic adjustment
    void AdjustBandwidthBasedOnConditions();
    void SetMaxBandwidth(uint32_t max_kbps);
    
    // Packet prioritization
    static PacketPriority GetPriorityForPacketType(uint16_t packet_type);
    
    // Traffic shaping
    bool ShouldDelayPacket(PacketPriority priority) const;
    uint32_t CalculateOptimalPacketSize() const;
    
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace P2P