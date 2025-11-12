#include <gtest/gtest.h>
#include "../include/ConfigManager.h"
#include "../include/Logger.h"
#include "../include/bandwidth/BandwidthManager.h"
#include "../include/SecurityManager.h"
#include "../include/NetworkManager.h"
#include <memory>

using namespace P2P;

// Test core functionality without Detours dependencies
class CoreFunctionalityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize logger
        LoggingConfig logger_config;
        logger_config.level = "debug";
        logger_config.file = "test_core.log";
        logger_config.console_output = true;
        Logger::GetInstance().Initialize(logger_config);
        
        // Initialize config manager
        config_mgr = &ConfigManager::GetInstance();
        config_mgr->LoadFromFile("test_config.json");
        
        // Initialize security manager
        security_mgr = std::make_unique<SecurityManager>();
        security_mgr->Initialize(true);
        
        // Initialize bandwidth manager
        bandwidth_mgr = std::make_unique<BandwidthManager>();