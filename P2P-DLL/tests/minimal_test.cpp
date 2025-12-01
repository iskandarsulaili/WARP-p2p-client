#include <gtest/gtest.h>
#include "ConfigManager.h"

using namespace P2P;

class ConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ConfigManagerTest, SimpleTest) {
    auto& config_mgr = ConfigManager::GetInstance();
    EXPECT_TRUE(true);
}