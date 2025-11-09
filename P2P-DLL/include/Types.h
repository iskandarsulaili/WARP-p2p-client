#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace P2P {

/**
 * Connection state for WebRTC peer connections
 */
enum class ConnectionState {
    NEW,
    CONNECTING,
    CONNECTED,
    DISCONNECTED,
    FAILED,
    CLOSED
};

/**
 * Route decision for packet routing
 */
enum class RouteDecision {
    P2P,        // Send via P2P
    SERVER,     // Send to centralized server
    BROADCAST,  // Broadcast to all peers
    DROP        // Drop packet
};

/**
 * Rate limit type
 */
enum class RateLimitType {
    API_CALL,
    WEBSOCKET_MESSAGE,
    P2P_PACKET
};

/**
 * Log level
 */
enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERR,  // Changed from ERROR to avoid Windows.h macro conflict
    FATAL
};

/**
 * Configuration structures
 */
struct CoordinatorConfig {
    std::string rest_api_url;
    std::string websocket_url;
    int timeout_ms;  // Timeout in milliseconds
    int timeout_seconds;  // Timeout in seconds (for compatibility)
    int reconnect_max_attempts;
    int reconnect_backoff_ms;
};

struct WebRTCConfig {
    std::vector<std::string> stun_servers;
    std::vector<std::string> turn_servers;
    std::string ice_transport_policy;
    std::string bundle_policy;
    std::string rtcp_mux_policy;
    bool enable_dtls;
    bool enable_rtp_data_channels;
};

struct P2PConfig {
    bool enabled;
    int max_peers;
    int max_packet_size_bytes;
    int max_bandwidth_mbps;
    int target_bitrate_kbps;
    bool enable_congestion_control;
    int packet_queue_size;
};

struct SecurityConfig {
    bool enable_encryption;
    bool encryption_enabled;  // Alias for enable_encryption (for compatibility)
    bool enable_authentication;
    std::string api_key;
    std::string jwt_token;
    bool certificate_validation;
    std::string tls_version;
};

struct LoggingConfig {
    std::string level;
    std::string file;
    int max_file_size_mb;
    int max_files;
    bool console_output;
    bool async_logging;
};

struct ZonesConfig {
    std::vector<std::string> p2p_enabled_zones;
    bool fallback_on_failure;
    int zone_transition_timeout_ms;
};

struct PerformanceConfig {
    int worker_threads;
    int io_thread_pool_size;
    bool enable_packet_batching;
    int packet_batch_size;
    int packet_batch_timeout_ms;
};

struct HostConfig {
    bool enable_hosting;
    int max_players;
    int max_zones;
    int heartbeat_interval_seconds;
    int quality_report_interval_seconds;
};

/**
 * Complete configuration
 */
struct Config {
    CoordinatorConfig coordinator;
    WebRTCConfig webrtc;
    P2PConfig p2p;
    SecurityConfig security;
    LoggingConfig logging;
    ZonesConfig zones;
    PerformanceConfig performance;
    HostConfig host;
};

/**
 * Packet structure
 */
struct Packet {
    uint16_t packet_id;
    uint16_t type;  // Packet type for routing decisions
    std::vector<uint8_t> data;
    size_t length;
};

/**
 * Peer information
 */
struct PeerInfo {
    std::string peer_id;
    std::string player_id;
    ConnectionState state;
    float latency_ms;
    float packet_loss_percent;
};

/**
 * Session information
 */
struct SessionInfo {
    std::string session_id;
    std::string zone_id;
    std::string host_id;
    std::vector<std::string> peer_ids;
    int max_players;
    int current_players;
};

} // namespace P2P

