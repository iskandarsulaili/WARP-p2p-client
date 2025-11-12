#include <gtest/gtest.h>
#include "../include/ConfigManager.h"
#include "../include/Logger.h"
#include <memory>

using namespace P2P;

// Simple test to verify basic compilation
TEST(SimpleTest, BasicCompilation) {
    // Test that we can create instances
    auto& logger = Logger::GetInstance();
    auto& config = ConfigManager::GetInstance();
    
    EXPECT_TRUE(true); // Basic test passes
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}