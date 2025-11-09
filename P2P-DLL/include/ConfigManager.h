#pragma once

#include "Types.h"
#include <string>
#include <memory>

namespace P2P {

/**
 * Configuration Manager
 * 
 * Loads and validates configuration from JSON file.
 * Provides access to configuration throughout the application.
 */
class ConfigManager {
public:
    /**
     * Get singleton instance
     */
    static ConfigManager& GetInstance();

    /**
     * Load configuration from file
     * 
     * @param config_path Path to configuration JSON file
     * @return true if loaded successfully, false otherwise
     */
    bool LoadFromFile(const std::string& config_path);

    /**
     * Load configuration from JSON string
     * 
     * @param json_str JSON configuration string
     * @return true if loaded successfully, false otherwise
     */
    bool LoadFromString(const std::string& json_str);

    /**
     * Validate configuration
     * 
     * @return true if configuration is valid, false otherwise
     */
    bool Validate() const;

    /**
     * Get configuration
     * 
     * @return Reference to configuration
     */
    const Config& GetConfig() const;

    /**
     * Get coordinator configuration
     */
    const CoordinatorConfig& GetCoordinatorConfig() const;

    /**
     * Get WebRTC configuration
     */
    const WebRTCConfig& GetWebRTCConfig() const;

    /**
     * Get P2P configuration
     */
    const P2PConfig& GetP2PConfig() const;

    /**
     * Get security configuration
     */
    const SecurityConfig& GetSecurityConfig() const;

    /**
     * Get logging configuration
     */
    const LoggingConfig& GetLoggingConfig() const;

    /**
     * Get zones configuration
     */
    const ZonesConfig& GetZonesConfig() const;

    /**
     * Get performance configuration
     */
    const PerformanceConfig& GetPerformanceConfig() const;

    /**
     * Get host configuration
     */
    const HostConfig& GetHostConfig() const;

    /**
     * Check if P2P is enabled
     */
    bool IsP2PEnabled() const;

    /**
     * Check if zone is P2P-enabled
     * 
     * @param zone_id Zone identifier
     * @return true if zone is P2P-enabled, false otherwise
     */
    bool IsZoneP2PEnabled(const std::string& zone_id) const;

    /**
     * Update JWT token
     *
     * @param token New JWT token
     */
    void UpdateJWTToken(const std::string& token);

    /**
     * Get coordinator REST API URL
     *
     * @return Coordinator URL
     */
    std::string GetCoordinatorUrl() const;

    /**
     * Get coordinator WebSocket signaling URL
     *
     * @return Signaling URL
     */
    std::string GetSignalingUrl() const;

    /**
     * Get API key for coordinator authentication
     *
     * @return API key
     */
    std::string GetApiKey() const;

private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    Config config_;
    bool loaded_ = false;
};

} // namespace P2P

