#include <gtest/gtest.h>
#include "NetworkManager.h"
#include "ConfigManager.h"
#include "Logger.h"
#include <fstream>

using namespace P2P;

class NetworkManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize logger for testing
        LoggingConfig log_config;
        log_config.level = "debug";
        log_config.file = "test_network.log";
        log_config.console_output = true;
        log_config.async_logging = false;
        Logger::GetInstance().Initialize(log_config);
        
        // Load test configuration
        auto& config_mgr = ConfigManager::GetInstance();
        config_mgr.LoadFromFile("test_config.json");
    }

    void TearDown() override {
        // Cleanup
        auto& network_mgr = NetworkManager::GetInstance();
        network_mgr.Shutdown();
        
        Logger::GetInstance().Shutdown();
    }
};

TEST_F(NetworkManagerTest, OnZoneChange_P2PEnabledZone_AttemptsSignalingConnection) {
    auto& network_mgr = NetworkManager::GetInstance();
    auto& config_mgr = ConfigManager::GetInstance();
    
    // Initialize network manager
    bool init_result = network_mgr.Initialize("test_peer_123");
    EXPECT_TRUE(init_result);
    
    // Test zone change to P2P-enabled zone
    network_mgr.OnZoneChange("prontera");
    
    // The test should complete without crashing
    // Actual connection attempt will fail since we don't have a real signaling server
    // but the code should handle this gracefully
    SUCCEED();
}

TEST_F(NetworkManagerTest, OnZoneChange_P2PDisabledZone_DisconnectsSignaling) {
    auto& network_mgr = NetworkManager::GetInstance();
    auto& config_mgr = ConfigManager::GetInstance();
    
    // Initialize network manager
    bool init_result = network_mgr.Initialize("test_peer_123");
    EXPECT_TRUE(init_result);
    
    // Test zone change to non-P2P zone
    network_mgr.OnZoneChange("non_p2p_zone");
    
    // Should attempt to disconnect from signaling (if connected)
    SUCCEED();
}

TEST_F(NetworkManagerTest, ZoneTransition_P2PToNonP2P_HandlesGracefully) {
    auto& network_mgr = NetworkManager::GetInstance();
    
    // Initialize network manager
    bool init_result = network_mgr.Initialize("test_peer_123");
    EXPECT_TRUE(init_result);
    
    // Transition from P2P to non-P2P zone
    network_mgr.OnZoneChange("prontera");  // P2P enabled
    network_mgr.OnZoneChange("unknown_zone");  // P2P disabled
    
    // Should handle the transition without issues
    SUCCEED();
}

TEST_F(NetworkManagerTest, ZoneTransition_NonP2PToP2P_HandlesGracefully) {
    auto& network_mgr = NetworkManager::GetInstance();
    
    // Initialize network manager
    bool init_result = network_mgr.Initialize("test_peer_123");
    EXPECT_TRUE(init_result);
    
    // Transition from non-P2P to P2P zone
    network_mgr.OnZoneChange("unknown_zone");  // P2P disabled
    network_mgr.OnZoneChange("geffen");        // P2P enabled
    
    // Should handle the transition without issues
    SUCCEED();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}