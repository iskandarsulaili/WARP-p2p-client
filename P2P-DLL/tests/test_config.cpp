#include <gtest/gtest.h>
#include "ConfigManager.h"
#include <fstream>

using namespace P2P;

class ConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset singleton state (for testing purposes)
        // Note: In production, singleton should not be reset
    }

    void TearDown() override {
        // Cleanup
    }
};

TEST_F(ConfigManagerTest, LoadFromFile_ValidConfig_ReturnsTrue) {
    auto& config_mgr = ConfigManager::GetInstance();
    
    bool result = config_mgr.LoadFromFile("test_config.json");
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(config_mgr.Validate());
}

TEST_F(ConfigManagerTest, LoadFromFile_InvalidPath_ReturnsFalse) {
    auto& config_mgr = ConfigManager::GetInstance();
    
    bool result = config_mgr.LoadFromFile("nonexistent_config.json");
    
    EXPECT_FALSE(result);
}

TEST_F(ConfigManagerTest, LoadFromString_ValidJSON_ReturnsTrue) {
    auto& config_mgr = ConfigManager::GetInstance();
    
    std::string json_config = R"({
        "coordinator": {
            "rest_api_url": "http://localhost:8001/api/v1",
            "websocket_url": "ws://localhost:8001/api/v1/signaling/ws"
        },
        "p2p": {
            "enabled": true,
            "max_peers": 50
        },
        "logging": {
            "level": "info",
            "file": "test.log"
        }
    })";
    
    bool result = config_mgr.LoadFromString(json_config);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(config_mgr.Validate());
}

TEST_F(ConfigManagerTest, LoadFromString_InvalidJSON_ReturnsFalse) {
    auto& config_mgr = ConfigManager::GetInstance();
    
    std::string invalid_json = "{ invalid json }";
    
    bool result = config_mgr.LoadFromString(invalid_json);
    
    EXPECT_FALSE(result);
}

TEST_F(ConfigManagerTest, GetCoordinatorConfig_ReturnsCorrectValues) {
    auto& config_mgr = ConfigManager::GetInstance();
    config_mgr.LoadFromFile("test_config.json");
    
    const auto& coord_config = config_mgr.GetCoordinatorConfig();
    
    EXPECT_EQ(coord_config.rest_api_url, "http://test-coordinator:8001/api/v1");
    EXPECT_EQ(coord_config.websocket_url, "ws://test-coordinator:8001/api/v1/signaling/ws");
    EXPECT_EQ(coord_config.timeout_seconds, 10);
    EXPECT_EQ(coord_config.reconnect_max_attempts, 3);
    EXPECT_EQ(coord_config.reconnect_backoff_ms, 500);
}

TEST_F(ConfigManagerTest, GetP2PConfig_ReturnsCorrectValues) {
    auto& config_mgr = ConfigManager::GetInstance();
    config_mgr.LoadFromFile("test_config.json");
    
    const auto& p2p_config = config_mgr.GetP2PConfig();
    
    EXPECT_TRUE(p2p_config.enabled);
    EXPECT_EQ(p2p_config.max_peers, 10);
    EXPECT_EQ(p2p_config.max_packet_size_bytes, 32768);
    EXPECT_EQ(p2p_config.max_bandwidth_mbps, 50);
}

TEST_F(ConfigManagerTest, GetSecurityConfig_ReturnsCorrectValues) {
    auto& config_mgr = ConfigManager::GetInstance();
    config_mgr.LoadFromFile("test_config.json");
    
    const auto& security_config = config_mgr.GetSecurityConfig();
    
    EXPECT_TRUE(security_config.enable_encryption);
    EXPECT_TRUE(security_config.enable_authentication);
    EXPECT_EQ(security_config.api_key, "test-api-key");
    EXPECT_EQ(security_config.jwt_token, "test-jwt-token");
    EXPECT_EQ(security_config.tls_version, "1.3");
}

TEST_F(ConfigManagerTest, IsP2PEnabled_ReturnsTrue) {
    auto& config_mgr = ConfigManager::GetInstance();
    config_mgr.LoadFromFile("test_config.json");
    
    EXPECT_TRUE(config_mgr.IsP2PEnabled());
}

TEST_F(ConfigManagerTest, IsZoneP2PEnabled_EnabledZone_ReturnsTrue) {
    auto& config_mgr = ConfigManager::GetInstance();
    config_mgr.LoadFromFile("test_config.json");
    
    EXPECT_TRUE(config_mgr.IsZoneP2PEnabled("test_zone_1"));
    EXPECT_TRUE(config_mgr.IsZoneP2PEnabled("test_zone_2"));
}

TEST_F(ConfigManagerTest, IsZoneP2PEnabled_DisabledZone_ReturnsFalse) {
    auto& config_mgr = ConfigManager::GetInstance();
    config_mgr.LoadFromFile("test_config.json");
    
    EXPECT_FALSE(config_mgr.IsZoneP2PEnabled("unknown_zone"));
}

TEST_F(ConfigManagerTest, UpdateJWTToken_UpdatesToken) {
    auto& config_mgr = ConfigManager::GetInstance();
    config_mgr.LoadFromFile("test_config.json");
    
    std::string new_token = "new-jwt-token-12345";
    config_mgr.UpdateJWTToken(new_token);
    
    const auto& security_config = config_mgr.GetSecurityConfig();
    EXPECT_EQ(security_config.jwt_token, new_token);
}

TEST_F(ConfigManagerTest, Validate_MissingCoordinatorURL_ReturnsFalse) {
    auto& config_mgr = ConfigManager::GetInstance();
    
    std::string invalid_config = R"({
        "coordinator": {
            "rest_api_url": "",
            "websocket_url": ""
        },
        "p2p": {
            "enabled": true,
            "max_peers": 50
        },
        "logging": {
            "file": "test.log"
        }
    })";
    
    bool result = config_mgr.LoadFromString(invalid_config);
    
    EXPECT_FALSE(result);
}

TEST_F(ConfigManagerTest, Validate_InvalidP2PConfig_ReturnsFalse) {
    auto& config_mgr = ConfigManager::GetInstance();
    
    std::string invalid_config = R"({
        "coordinator": {
            "rest_api_url": "http://localhost:8001/api/v1",
            "websocket_url": "ws://localhost:8001/api/v1/signaling/ws"
        },
        "p2p": {
            "enabled": true,
            "max_peers": -1
        },
        "logging": {
            "file": "test.log"
        }
    })";
    
    bool result = config_mgr.LoadFromString(invalid_config);
    
    EXPECT_FALSE(result);
}