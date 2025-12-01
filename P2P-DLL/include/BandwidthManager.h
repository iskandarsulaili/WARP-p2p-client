#pragma once

#include "Types.h"
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include <atomic>
#include <functional>

namespace P2P {

/**
 * BandwidthManager - Manages bandwidth optimization and adaptive bitrate control
 */
class BandwidthManager {
public:
    BandwidthManager();
    ~BandwidthManager();

    // Disable copy and move
    BandwidthManager(const BandwidthManager&) = delete;
    BandwidthManager& operator=(const BandwidthManager&) = delete;
    BandwidthManager(BandwidthManager&&) = delete;
    BandwidthManager& operator=(BandwidthManager&&) = delete;

    /**
     * Initialize the bandwidth manager with configuration
     * @param config Bandwidth configuration
     * @return true if initialization succeeded
     */
    bool Initialize(const BandwidthConfig& config);

    /**
     * Shutdown the bandwidth manager
     */
    void Shutdown();

    /**
     * Update bandwidth metrics for sent packet
     * @param peer_id The peer ID
     * @param packet_size Size of the packet in bytes
     * @param priority Packet priority
     */
    void UpdateSentMetrics(const std::string& peer_id, size_t packet_size, PacketPriority priority);

    /**
     * Update bandwidth metrics for received packet
     * @param peer_id The peer ID
     * @param packet_size Size of the packet in bytes
     */
    void UpdateReceivedMetrics(const std::string& peer_id, size_t packet_size);

    /**
     * Update packet loss metrics
     * @param peer_id The peer ID
     * @param packets_lost Number of packets lost
     */
    void UpdatePacketLoss(const std::string& peer_id, uint64_t packets_lost);

    /**
     * Update latency metrics
     * @param peer_id The peer ID
     * @param latency_ms Latency in milliseconds
     */
    void UpdateLatency(const std::string& peer_id, float latency_ms);

    /**
     * Get current recommended bitrate for a peer
     * @param peer_id The peer ID
     * @return Recommended bitrate in kbps
     */
    float GetRecommendedBitrate(const std::string& peer_id) const;

    /**
     * Check if congestion is detected for a peer
     * @param peer_id The peer ID
     * @return true if congestion is detected
     */
    bool IsCongested(const std::string& peer_id) const;

    /**
     * Get packet priority based on packet type
     * @param packet_type RO packet type
     * @return Packet priority level
     */
    static PacketPriority GetPacketPriority(uint16_t packet_type);

    /**
     * Should packet be dropped due to congestion
     * @param priority Packet priority
     * @param current_congestion Current congestion level
     * @return true if packet should be dropped
     */
    bool ShouldDropPacket(PacketPriority priority, float current_congestion) const;

    /**
     * Get bandwidth metrics for a peer
     * @param peer_id The peer ID
     * @return Bandwidth metrics
     */
    BandwidthMetrics GetMetrics(const std::string& peer_id) const;

    /**
     * Get overall bandwidth metrics
     * @return Aggregate bandwidth metrics
     */
    BandwidthMetrics GetOverallMetrics() const;

    /**
     * Reset metrics for a peer
     * @param peer_id The peer ID
     */
    void ResetMetrics(const std::string& peer_id);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace P2P