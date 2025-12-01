#include "../../include/BandwidthManager.h"
#include "../../include/Logger.h"
#include <algorithm>
#include <cmath>
#include <thread>
#include <sstream>
#include <iomanip>
#include <map>
#include <array>

namespace P2P {

// Implementation details
struct BandwidthManager::Impl {
    BandwidthConfig config;
    std::map<std::string, BandwidthMetrics> peer_metrics;
    std::array<std::atomic<uint64_t>, 5> priority_bytes_sent{};
    std::array<std::atomic<uint64_t>, 5> priority_packets_sent{};
    std::mutex mutex;
    std::atomic<bool> initialized{false};
};

BandwidthManager::BandwidthManager() : impl_(std::make_unique<Impl>()) {
    // Initialize atomic arrays
    for (auto& count : impl_->priority_bytes_sent) {
        count.store(0);
    }
    for (auto& count : impl_->priority_packets_sent) {
        count.store(0);
    }
}

BandwidthManager::~BandwidthManager() = default;

bool BandwidthManager::Initialize(const BandwidthConfig& config) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->config = config;
    impl_->initialized = true;
    
    LOG_INFO("BandwidthManager initialized");
    return true;
}

void BandwidthManager::Shutdown() {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->initialized = false;
    impl_->peer_metrics.clear();
    
    // Reset atomic counters
    for (auto& count : impl_->priority_bytes_sent) {
        count.store(0);
    }
    for (auto& count : impl_->priority_packets_sent) {
        count.store(0);
    }
}

void BandwidthManager::UpdateSentMetrics(const std::string& peer_id, size_t packet_size, PacketPriority priority) {
    if (!impl_->initialized) return;

    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    // Update peer metrics
    auto& metrics = impl_->peer_metrics[peer_id];
    metrics.bytes_sent += packet_size;
    metrics.packets_sent++;
    metrics.last_update = std::chrono::steady_clock::now();
    
    // Update priority counters
    size_t priority_idx = static_cast<size_t>(priority);
    if (priority_idx < impl_->priority_bytes_sent.size()) {
        impl_->priority_bytes_sent[priority_idx] += packet_size;
        impl_->priority_packets_sent[priority_idx]++;
    }
}

void BandwidthManager::UpdateReceivedMetrics(const std::string& peer_id, size_t packet_size) {
    if (!impl_->initialized) return;

    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    auto& metrics = impl_->peer_metrics[peer_id];
    metrics.bytes_received += packet_size;
    metrics.packets_received++;
    metrics.last_update = std::chrono::steady_clock::now();
}

void BandwidthManager::UpdatePacketLoss(const std::string& peer_id, uint64_t packets_lost) {
    if (!impl_->initialized) return;

    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    auto& metrics = impl_->peer_metrics[peer_id];
    metrics.packets_lost += packets_lost;
    metrics.last_update = std::chrono::steady_clock::now();
    
    // Calculate packet loss percentage
    uint64_t total_packets = metrics.packets_received + metrics.packets_lost;
    if (total_packets > 0) {
        metrics.packet_loss_percent = (static_cast<float>(metrics.packets_lost) / total_packets) * 100.0f;
    }
}

void BandwidthManager::UpdateLatency(const std::string& peer_id, float latency_ms) {
    if (!impl_->initialized) return;

    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    auto& metrics = impl_->peer_metrics[peer_id];
    
    // Simple exponential moving average for latency
    if (metrics.average_latency_ms == 0.0f) {
        metrics.average_latency_ms = latency_ms;
    } else {
        metrics.average_latency_ms = 0.8f * metrics.average_latency_ms + 0.2f * latency_ms;
    }
    metrics.last_update = std::chrono::steady_clock::now();
}

float BandwidthManager::GetRecommendedBitrate(const std::string& peer_id) const {
    if (!impl_->initialized) return 0.0f;

    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    const auto it = impl_->peer_metrics.find(peer_id);
    if (it == impl_->peer_metrics.end()) {
        return impl_->config.target_bitrate_kbps;
    }
    
    const auto& metrics = it->second;
    
    // Simple adaptive bitrate algorithm
    float base_bitrate = 1000.0f; // Default target bitrate
    
    // Reduce bitrate based on packet loss
    if (metrics.packet_loss_percent > 5.0f) {
        base_bitrate *= 0.7f; // Reduce by 30% for high packet loss
    } else if (metrics.packet_loss_percent > 2.0f) {
        base_bitrate *= 0.85f; // Reduce by 15% for moderate packet loss
    }
    
    // Reduce bitrate based on high latency
    if (metrics.average_latency_ms > 200.0f) {
        base_bitrate *= 0.6f; // Reduce by 40% for very high latency
    } else if (metrics.average_latency_ms > 100.0f) {
        base_bitrate *= 0.8f; // Reduce by 20% for high latency
    }
    
    // Ensure minimum bitrate
    return std::max(base_bitrate, 100.0f);
}

bool BandwidthManager::IsCongested(const std::string& peer_id) const {
    if (!impl_->initialized) return false;

    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    const auto it = impl_->peer_metrics.find(peer_id);
    if (it == impl_->peer_metrics.end()) {
        return false;
    }
    
    const auto& metrics = it->second;
    
    // Congestion detection based on packet loss and latency
    return metrics.packet_loss_percent > 10.0f || metrics.average_latency_ms > 300.0f;
}

PacketPriority BandwidthManager::GetPacketPriority(uint16_t packet_type) {
    // Map RO packet types to priorities
    switch (packet_type) {
        case 0x0089: // Movement
        case 0x009F: // Item pickup
            return PacketPriority::CRITICAL;
        
        case 0x008C: // Chat
        case 0x00A2: // Skill use
            return PacketPriority::HIGH;
        
        case 0x00A7: // Emotion
        case 0x00B0: // Party info
            return PacketPriority::NORMAL;
        
        default:
            return PacketPriority::LOW;
    }
}

bool BandwidthManager::ShouldDropPacket(PacketPriority priority, float current_congestion) const {
    if (!impl_->initialized) return false;

    // Drop packets based on priority and congestion level
    switch (priority) {
        case PacketPriority::CRITICAL:
            return false; // Never drop critical packets
            
        case PacketPriority::HIGH:
            return current_congestion > 0.8f; // Drop only under extreme congestion
            
        case PacketPriority::NORMAL:
            return current_congestion > 0.6f;
            
        case PacketPriority::LOW:
        case PacketPriority::BACKGROUND:
            return current_congestion > 0.4f;
            
        default:
            return false;
    }
}

BandwidthMetrics BandwidthManager::GetMetrics(const std::string& peer_id) const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    const auto it = impl_->peer_metrics.find(peer_id);
    if (it != impl_->peer_metrics.end()) {
        return it->second;
    }
    
    return BandwidthMetrics{};
}

BandwidthMetrics BandwidthManager::GetOverallMetrics() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    BandwidthMetrics overall;
    for (const auto& [peer_id, metrics] : impl_->peer_metrics) {
        overall.bytes_sent += metrics.bytes_sent;
        overall.bytes_received += metrics.bytes_received;
        overall.packets_sent += metrics.packets_sent;
        overall.packets_received += metrics.packets_received;
        overall.packets_lost += metrics.packets_lost;
    }
    
    // Calculate averages
    if (!impl_->peer_metrics.empty()) {
        float total_latency = 0.0f;
        float total_loss = 0.0f;
        
        for (const auto& [peer_id, metrics] : impl_->peer_metrics) {
            total_latency += metrics.average_latency_ms;
            total_loss += metrics.packet_loss_percent;
        }
        
        overall.average_latency_ms = total_latency / impl_->peer_metrics.size();
        overall.packet_loss_percent = total_loss / impl_->peer_metrics.size();
    }
    
    overall.last_update = std::chrono::steady_clock::now();
    return overall;
}

void BandwidthManager::ResetMetrics(const std::string& peer_id) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    if (peer_id.empty()) {
        // Reset all metrics
        for (auto& [id, metrics] : impl_->peer_metrics) {
            metrics = BandwidthMetrics{};
            metrics.last_update = std::chrono::steady_clock::now();
        }
    } else {
        // Reset specific peer metrics
        auto it = impl_->peer_metrics.find(peer_id);
        if (it != impl_->peer_metrics.end()) {
            it->second = BandwidthMetrics{};
            it->second.last_update = std::chrono::steady_clock::now();
        }
    }
}

} // namespace P2P