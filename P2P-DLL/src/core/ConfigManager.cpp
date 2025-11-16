#include "ConfigManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>

using json = nlohmann::json;

namespace P2P {

ConfigManager& ConfigManager::GetInstance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::LoadFromFile(const std::string& config_path) {
    try {
        std::ifstream file(config_path);
        if (!file.is_open()) {
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return LoadFromString(buffer.str());
    }
    catch (const std::exception&) {
        return false;
    }
}

bool ConfigManager::LoadFromString(const std::string& json_str) {
    try {
        auto j = json::parse(json_str);

        // Parse coordinator config
        if (j.contains("coordinator")) {
            auto& coord = j["coordinator"];
            config_.coordinator.rest_api_url = coord.value("rest_api_url", "http://localhost:8001/api/v1");
            config_.coordinator.websocket_url = coord.value("websocket_url", "ws://localhost:8001/api/v1/signaling/ws");
            config_.coordinator.timeout_seconds = coord.value("timeout_seconds", 30);
            config_.coordinator.reconnect_max_attempts = coord.value("reconnect_max_attempts", 5);
            config_.coordinator.reconnect_backoff_ms = coord.value("reconnect_backoff_ms", 1000);
        }

        // Parse WebRTC config
        if (j.contains("webrtc")) {
            auto& webrtc = j["webrtc"];
            if (webrtc.contains("stun_servers")) {
                config_.webrtc.stun_servers = webrtc["stun_servers"].get<std::vector<std::string>>();
            }
            if (webrtc.contains("turn_servers")) {
                config_.webrtc.turn_servers = webrtc["turn_servers"].get<std::vector<std::string>>();
            }
            config_.webrtc.ice_transport_policy = webrtc.value("ice_transport_policy", "all");
            config_.webrtc.bundle_policy = webrtc.value("bundle_policy", "balanced");
            config_.webrtc.rtcp_mux_policy = webrtc.value("rtcp_mux_policy", "require");
            config_.webrtc.enable_dtls = webrtc.value("enable_dtls", true);
            config_.webrtc.enable_rtp_data_channels = webrtc.value("enable_rtp_data_channels", false);
        }

            // Parse P2P config
            if (j.contains("p2p")) {
                auto& p2p = j["p2p"];
                config_.p2p.enabled = p2p.value("enabled", true);
                config_.p2p.max_peers = p2p.value("max_peers", 50);
                config_.p2p.max_packet_size_bytes = p2p.value("max_packet_size_bytes", 65536);
                config_.p2p.max_bandwidth_mbps = p2p.value("max_bandwidth_mbps", 100);
                config_.p2p.target_bitrate_kbps = p2p.value("target_bitrate_kbps", 5000);
                config_.p2p.enable_congestion_control = p2p.value("enable_congestion_control", true);
                config_.p2p.packet_queue_size = p2p.value("packet_queue_size", 1000);
                // Mesh/AOI extensions
                config_.p2p.aoi_radius = p2p.value("aoi_radius", 100.0f);
                config_.p2p.mesh_refresh_interval_ms = p2p.value("mesh_refresh_interval_ms", 5000);
                config_.p2p.peer_score_threshold = p2p.value("peer_score_threshold", 0.5f);
                config_.p2p.prune_interval_ms = p2p.value("prune_interval_ms", 10000);
            }

        if (j.contains("bandwidth")) {
            auto& bandwidth = j["bandwidth"];
            config_.bandwidth.bandwidth_update_interval_ms = bandwidth.value("bandwidth_update_interval_ms", 1000);
            config_.bandwidth.congestion_threshold_percent = bandwidth.value("congestion_threshold_percent", 70.0f);
            config_.bandwidth.min_bitrate_kbps = bandwidth.value("min_bitrate_kbps", 100.0f);
            config_.bandwidth.max_bitrate_kbps = bandwidth.value("max_bitrate_kbps", 10000.0f);
            config_.bandwidth.enable_adaptive_bitrate = bandwidth.value("enable_adaptive_bitrate", true);
            config_.bandwidth.packet_priority_enabled = bandwidth.value("packet_priority_enabled", true);
        }

        // Parse compression config
        if (j.contains("compression")) {
            auto& compression = j["compression"];
            config_.compression.enabled = compression.value("enabled", true);
            config_.compression.algorithm = compression.value("algorithm", "lz4");
            config_.compression.compression_level = compression.value("compression_level", 6);
            config_.compression.min_size_for_compression = compression.value("min_size_for_compression", 100);
            config_.compression.compression_ratio_threshold = compression.value("compression_ratio_threshold", 0.8f);
            config_.compression.enable_metrics = compression.value("enable_metrics", true);
        }

            // Parse security config
            if (j.contains("security")) {
                auto& security = j["security"];
                config_.security.enable_encryption = security.value("enable_encryption", true);
                config_.security.enable_authentication = security.value("enable_authentication", true);
                config_.security.api_key = security.value("api_key", "");
                config_.security.jwt_token = security.value("jwt_token", "");
                config_.security.certificate_validation = security.value("certificate_validation", true);
                config_.security.tls_version = security.value("tls_version", "1.3");
                // ED25519 signature extensions
                config_.security.ed25519_private_key_path = security.value("ed25519_private_key_path", "");
                config_.security.enable_signature = security.value("enable_signature", true);
            }

        // Parse logging config
        if (j.contains("logging")) {
            auto& logging = j["logging"];
            config_.logging.level = logging.value("level", "info");
            config_.logging.file = logging.value("file", "p2p_dll.log");
            config_.logging.max_file_size_mb = logging.value("max_file_size_mb", 10);
            config_.logging.max_files = logging.value("max_files", 5);
            config_.logging.console_output = logging.value("console_output", true);
            config_.logging.async_logging = logging.value("async_logging", true);
        }

            // Parse zones config
            if (j.contains("zones")) {
                auto& zones = j["zones"];
                if (zones.contains("p2p_enabled_zones")) {
                    config_.zones.p2p_enabled_zones = zones["p2p_enabled_zones"].get<std::vector<std::string>>();
                }
                config_.zones.fallback_on_failure = zones.value("fallback_on_failure", true);
                config_.zones.zone_transition_timeout_ms = zones.value("zone_transition_timeout_ms", 5000);
                // Per-zone max peers
                if (zones.contains("max_peers_per_zone")) {
                    for (auto& el : zones["max_peers_per_zone"].items()) {
                        config_.zones.max_peers_per_zone[el.key()] = el.value().get<int>();
                    }
                }
            }

        // Parse performance config
        if (j.contains("performance")) {
            auto& perf = j["performance"];
            config_.performance.worker_threads = perf.value("worker_threads", 4);
            config_.performance.io_thread_pool_size = perf.value("io_thread_pool_size", 2);
            config_.performance.enable_packet_batching = perf.value("enable_packet_batching", true);
            config_.performance.packet_batch_size = perf.value("packet_batch_size", 10);
            config_.performance.packet_batch_timeout_ms = perf.value("packet_batch_timeout_ms", 5);
        }

        // Parse host config
        if (j.contains("host")) {
            auto& host = j["host"];
            config_.host.enable_hosting = host.value("enable_hosting", false);
            config_.host.max_players = host.value("max_players", 50);
            config_.host.max_zones = host.value("max_zones", 5);
            config_.host.heartbeat_interval_seconds = host.value("heartbeat_interval_seconds", 30);
            config_.host.quality_report_interval_seconds = host.value("quality_report_interval_seconds", 60);
        }

        loaded_ = true;
        return Validate();
    }
    catch (const std::exception&) {
        return false;
    }
}

bool ConfigManager::Validate() const {
    if (!loaded_) {
        return false;
    }

    // Validate coordinator config
    if (config_.coordinator.rest_api_url.empty() || config_.coordinator.websocket_url.empty()) {
        return false;
    }

    // Validate P2P config
    if (config_.p2p.max_peers <= 0 || config_.p2p.max_packet_size_bytes <= 0) {
        return false;
    }

    // Validate logging config
    if (config_.logging.file.empty()) {
        return false;
    }

    return true;
}

const Config& ConfigManager::GetConfig() const {
    return config_;
}

const CoordinatorConfig& ConfigManager::GetCoordinatorConfig() const {
    return config_.coordinator;
}

const WebRTCConfig& ConfigManager::GetWebRTCConfig() const {
    return config_.webrtc;
}

const P2PConfig& ConfigManager::GetP2PConfig() const {
    return config_.p2p;
}

const SecurityConfig& ConfigManager::GetSecurityConfig() const {
    return config_.security;
}

const LoggingConfig& ConfigManager::GetLoggingConfig() const {
    return config_.logging;
}

const ZonesConfig& ConfigManager::GetZonesConfig() const {
    return config_.zones;
}

const PerformanceConfig& ConfigManager::GetPerformanceConfig() const {
    return config_.performance;
}

const BandwidthConfig& ConfigManager::GetBandwidthConfig() const {
    return config_.bandwidth;
}

const CompressionConfig& ConfigManager::GetCompressionConfig() const {
    return config_.compression;
}

const HostConfig& ConfigManager::GetHostConfig() const {
    return config_.host;
}

bool ConfigManager::IsP2PEnabled() const {
    return config_.p2p.enabled;
}

bool ConfigManager::IsZoneP2PEnabled(const std::string& zone_id) const {
    const auto& zones = config_.zones.p2p_enabled_zones;
    return std::find(zones.begin(), zones.end(), zone_id) != zones.end();
}

void ConfigManager::UpdateJWTToken(const std::string& token) {
    config_.security.jwt_token = token;
}

} // namespace P2P

