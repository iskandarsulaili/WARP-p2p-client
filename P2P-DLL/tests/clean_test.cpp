#include <gtest/gtest.h>
#include "ConfigManager.h"

using namespace P2P;

class CleanTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(CleanTest, BasicTest) {
    EXPECT_TRUE(true);
}