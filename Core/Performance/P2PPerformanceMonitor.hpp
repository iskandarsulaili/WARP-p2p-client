#ifndef P2P_PERFORMANCE_MONITOR_HPP
#define P2P_PERFORMANCE_MONITOR_HPP

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <chrono>
#include <cstdint>
#include <functional>

/**
 * @brief P2P Performance Monitor
 * 
 * Monitors and tracks performance metrics for P2P connections including:
 * - Latency (RTT)
 * - Throughput (bandwidth)
 * - Packet loss
 * - Connection quality
 * - Resource usage
 * 
 * Thread Safety: All public methods are thread-safe
 * 
 * Usage:
 *   P2PPerformanceMonitor monitor;
 *   monitor.start_monitoring();
 *   monitor.record_latency(peer_id, latency_ms);
 *   auto metrics = monitor.get_metrics(peer_id);
 */
class P2PPerformanceMonitor {
public:
    /**
     * @brief Performance metrics for a peer
     */
    struct PeerMetrics {
        std::string peer_id;
        
        // Latency metrics (milliseconds)
        double avg_latency_ms;
        double min_latency_ms;
        double max_latency_ms;
        double current_latency_ms;
        
        // Throughput metrics (bytes/second)
        uint64_t bytes_sent_per_sec;
        uint64_t bytes_received_per_sec;
        uint64_t total_bytes_sent;
        uint64_t total_bytes_received;
        
        // Packet metrics
        uint64_t packets_sent;
        uint64_t packets_received;
        uint64_t packets_lost;
        double packet_loss_rate;
        
        // Connection quality (0-100)
        uint32_t connection_quality;
        
        // Jitter (milliseconds)
        double jitter_ms;
        
        // Timestamps
        uint64_t monitoring_start_time;
        uint64_t last_update_time;
    };
    
    /**
     * @brief System resource metrics
     */
    struct ResourceMetrics {
        double cpu_usage_percent;
        uint64_t memory_usage_bytes;
        uint32_t active_connections;
        uint32_t total_threads;
        double network_utilization_percent;
    };
    
    /**
     * @brief Performance thresholds for alerts
     */
    struct PerformanceThresholds {
        double max_latency_ms;
        double max_jitter_ms;
        double max_packet_loss_rate;
        uint64_t min_throughput_bps;
        uint32_t min_connection_quality;
    };
    
    // Callback type definitions
    using ThresholdExceededCallback = std::function<void(const std::string& peer_id, const std::string& metric, double value)>;
    using MetricsUpdateCallback = std::function<void(const std::string& peer_id, const PeerMetrics& metrics)>;
    
    /**
     * @brief Constructor
     */
    P2PPerformanceMonitor();
    
    /**
     * @brief Destructor
     */
    ~P2PPerformanceMonitor();
    
    // Disable copy
    P2PPerformanceMonitor(const P2PPerformanceMonitor&) = delete;
    P2PPerformanceMonitor& operator=(const P2PPerformanceMonitor&) = delete;
    
    /**
     * @brief Start monitoring
     * @param thresholds Performance thresholds for alerts
     */
    void start_monitoring(const PerformanceThresholds& thresholds = {});
    
    /**
     * @brief Stop monitoring
     */
    void stop_monitoring();
    
    /**
     * @brief Record latency measurement
     * @param peer_id Peer ID
     * @param latency_ms Latency in milliseconds
     */
    void record_latency(const std::string& peer_id, double latency_ms);
    
    /**
     * @brief Record data sent
     * @param peer_id Peer ID
     * @param bytes Number of bytes sent
     */
    void record_data_sent(const std::string& peer_id, uint64_t bytes);
    
    /**
     * @brief Record data received
     * @param peer_id Peer ID
     * @param bytes Number of bytes received
     */
    void record_data_received(const std::string& peer_id, uint64_t bytes);
    
    /**
     * @brief Record packet sent
     * @param peer_id Peer ID
     */
    void record_packet_sent(const std::string& peer_id);
    
    /**
     * @brief Record packet received
     * @param peer_id Peer ID
     */
    void record_packet_received(const std::string& peer_id);
    
    /**
     * @brief Record packet lost
     * @param peer_id Peer ID
     */
    void record_packet_lost(const std::string& peer_id);
    
    /**
     * @brief Get metrics for a peer
     * @param peer_id Peer ID
     * @return Peer metrics or nullptr if not found
     */
    std::unique_ptr<PeerMetrics> get_metrics(const std::string& peer_id) const;
    
    /**
     * @brief Get metrics for all peers
     */
    std::vector<PeerMetrics> get_all_metrics() const;
    
    /**
     * @brief Get system resource metrics
     */
    ResourceMetrics get_resource_metrics() const;
    
    /**
     * @brief Update - call from main loop
     */
    void update();
    
    /**
     * @brief Reset metrics for a peer
     * @param peer_id Peer ID
     */
    void reset_peer_metrics(const std::string& peer_id);
    
    /**
     * @brief Reset all metrics
     */
    void reset_all_metrics();
    
    /**
     * @brief Export metrics to JSON
     * @return JSON string with all metrics
     */
    std::string export_metrics_json() const;
    
    /**
     * @brief Export metrics to CSV
     * @return CSV string with all metrics
     */
    std::string export_metrics_csv() const;
    
    // Event callbacks
    void on_threshold_exceeded(ThresholdExceededCallback callback) { threshold_callback_ = callback; }
    void on_metrics_update(MetricsUpdateCallback callback) { metrics_callback_ = callback; }

private:
    bool monitoring_;
    PerformanceThresholds thresholds_;
    
    // Peer metrics storage
    std::map<std::string, PeerMetrics> peer_metrics_;
    
    // Latency history for jitter calculation
    std::map<std::string, std::vector<double>> latency_history_;
    
    // Throughput tracking
    struct ThroughputTracker {
        uint64_t bytes_sent_last_second;
        uint64_t bytes_received_last_second;
        uint64_t last_update_time;
    };
    std::map<std::string, ThroughputTracker> throughput_trackers_;
    
    // Thread safety
    mutable std::mutex mutex_;
    
    // Callbacks
    ThresholdExceededCallback threshold_callback_;
    MetricsUpdateCallback metrics_callback_;
    
    // Internal methods
    PeerMetrics& get_or_create_metrics(const std::string& peer_id);
    void calculate_throughput(const std::string& peer_id);
    void calculate_jitter(const std::string& peer_id);
    void calculate_connection_quality(const std::string& peer_id);
    void check_thresholds(const std::string& peer_id);
    uint64_t get_current_time_ms() const;
    
    void log_info(const std::string& message);
    void log_warning(const std::string& message);
};

#endif // P2P_PERFORMANCE_MONITOR_HPP

