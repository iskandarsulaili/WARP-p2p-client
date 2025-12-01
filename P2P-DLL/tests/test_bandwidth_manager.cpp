#include "../../include/bandwidth/BandwidthManager.h"
#include "../../include/Logger.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>

namespace P2P {

class BandwidthManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize logger for testing
        LoggingConfig log_config;
        log_config.level = "debug";
        log_config.file = "test_bandwidth.log";
        log_config.console_output = true;
        Logger::GetInstance().Initialize(log_config);
        
        // Default configuration for tests
        config.critical_reserve_kbps = 128;
        config.high_priority_kbps = 256;
        config.normal_priority_kbps = 512;
        config.low_priority_kbps = 768;
        config.max_packet_size = 1400;
        config.min_packet_size = 64;
        
        ASSERT_TRUE(bw_manager.Initialize(config));
    }
    
    void TearDown() override {
        bw_manager.Shutdown();
    }
    
    EnhancedBandwidthConfig config;
    BandwidthManager bw_manager;
};

TEST_F(BandwidthManagerTest, Initialization) {
    BandwidthManager manager;
    EnhancedBandwidthConfig test_config;
    
    // Test valid configuration
    test_config.critical_reserve_kbps = 128;
    test_config.high_priority_kbps = 256;
    test_config.normal_priority_kbps = 512;
    test_config.low_priority_kbps = 768;
    test_config.max_packet_size = 1400;
    test_config.min_packet_size = 64;
    EXPECT_TRUE(manager.Initialize(test_config));
    
    // Test invalid configuration
    test_config.max_packet_size = 0;
    EXPECT_FALSE(manager.Initialize(test_config));
}

TEST_F(BandwidthManagerTest, PacketPriorityClassification) {
    // Test critical packets
    EXPECT_EQ(BandwidthManager::GetPriorityForPacketType(0x0089), PacketPriority::CRITICAL); // Movement
    EXPECT_EQ(BandwidthManager::GetPriorityForPacketType(0x0090), PacketPriority::CRITICAL); // Attack
    
    // Test high priority packets
    EXPECT_EQ(BandwidthManager::GetPriorityForPacketType(0x008C), PacketPriority::HIGH); // Chat
    EXPECT_EQ(BandwidthManager::GetPriorityForPacketType(0x0094), PacketPriority::HIGH); // NPC interaction
    
    // Test normal priority packets
    EXPECT_EQ(BandwidthManager::GetPriorityForPacketType(0x008D), PacketPriority::NORMAL); // Global chat
    EXPECT_EQ(BandwidthManager::GetPriorityForPacketType(0x008E), PacketPriority::NORMAL); // Party chat
    
    // Test low priority packets
    EXPECT_EQ(BandwidthManager::GetPriorityForPacketType(0x00B0), PacketPriority::LOW); // Status effects
    
    // Test background priority for unknown packets
    EXPECT_EQ(BandwidthManager::GetPriorityForPacketType(0xFFFF), PacketPriority::BACKGROUND);
}

TEST_F(BandwidthManagerTest, BandwidthAllocation) {
    // Test that we can send critical packets
    EXPECT_TRUE(bw_manager.CanSendPacket(PacketPriority::CRITICAL, 100));
    
    // Send a packet and check bandwidth usage
    bw_manager.PacketSent(PacketPriority::CRITICAL, 100);
    uint32_t usage = bw_manager.GetCurrentUsage(PacketPriority::CRITICAL);
    EXPECT_GT(usage, 0);
}

TEST_F(BandwidthManagerTest, CongestionDetection) {
    // Simulate high bandwidth usage
    for (int i = 0; i < 100; ++i) {
        bw_manager.PacketSent(PacketPriority::NORMAL, 1000); // 1KB packets
    }
    
    // Should detect congestion
    EXPECT_TRUE(bw_manager.IsCongested());
    
    // Critical packets should still be allowed
    EXPECT_TRUE(bw_manager.CanSendPacket(PacketPriority::CRITICAL, 100));
    
    // Non-critical packets should be blocked
    EXPECT_FALSE(bw_manager.CanSendPacket(PacketPriority::NORMAL, 100));
}

TEST_F(BandwidthManagerTest, QoSStatistics) {
    // Update latency and packet loss
    bw_manager.UpdateLatency(50.0);
    bw_manager.UpdatePacketLoss(0.05);
    
    const auto& stats = bw_manager.GetQoSStats();
    EXPECT_DOUBLE_EQ(stats.latency_ms, 50.0);
    EXPECT_DOUBLE_EQ(stats.packet_loss_rate, 0.05);
    EXPECT_GT(stats.jitter_ms, 0.0);
}

TEST_F(BandwidthManagerTest, DynamicBandwidthAdjustment) {
    // Simulate poor network conditions
    bw_manager.UpdateLatency(200.0);
    bw_manager.UpdatePacketLoss(0.2);
    
    // Should automatically adjust bandwidth
    bw_manager.AdjustBandwidthBasedOnConditions();
    
    // Bandwidth should be reduced (check congestion state instead)
    EXPECT_TRUE(bw_manager.IsCongested());
}

TEST_F(BandwidthManagerTest, PacketSizeValidation) {
    // Valid packet size
    EXPECT_TRUE(bw_manager.CanSendPacket(PacketPriority::NORMAL, config.max_packet_size));
    
    // Packet too large
    EXPECT_FALSE(bw_manager.CanSendPacket(PacketPriority::NORMAL, config.max_packet_size + 1));
    
    // Packet too small
    EXPECT_TRUE(bw_manager.CanSendPacket(PacketPriority::NORMAL, config.min_packet_size));
}

TEST_F(BandwidthManagerTest, PriorityBasedAccess) {
    // Fill up bandwidth with low priority packets
    for (int i = 0; i < 50; ++i) {
        bw_manager.PacketSent(PacketPriority::LOW, 1000);
    }
    
    // High priority should still have access
    EXPECT_TRUE(bw_manager.CanSendPacket(PacketPriority::HIGH, 100));
    
    // Critical priority should definitely have access
    EXPECT_TRUE(bw_manager.CanSendPacket(PacketPriority::CRITICAL, 100));
}

TEST_F(BandwidthManagerTest, OptimalPacketSizeCalculation) {
    // Good network - larger packets
    bw_manager.UpdateLatency(20.0);
    bw_manager.UpdatePacketLoss(0.01);
    uint32_t size1 = bw_manager.CalculateOptimalPacketSize();
    EXPECT_GE(size1, config.min_packet_size);
    EXPECT_LE(size1, config.max_packet_size);
    
    // Poor network - smaller packets
    bw_manager.UpdateLatency(100.0);
    bw_manager.UpdatePacketLoss(0.1);
    uint32_t size2 = bw_manager.CalculateOptimalPacketSize();
    EXPECT_GE(size2, config.min_packet_size);
    EXPECT_LE(size2, config.max_packet_size);
}

TEST_F(BandwidthManagerTest, PacketDelayDecision) {
    // Good conditions - no delay
    bw_manager.UpdateLatency(30.0);
    bw_manager.UpdatePacketLoss(0.02);
    EXPECT_FALSE(bw_manager.ShouldDelayPacket(PacketPriority::NORMAL));
    
    // Poor conditions - delay non-critical packets
    for (int i = 0; i < 100; ++i) {
        bw_manager.PacketSent(PacketPriority::NORMAL, 1000);
    }
    EXPECT_TRUE(bw_manager.ShouldDelayPacket(PacketPriority::NORMAL));
    
    // Critical packets should never be delayed
    EXPECT_FALSE(bw_manager.ShouldDelayPacket(PacketPriority::CRITICAL));
}

} // namespace P2P

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}