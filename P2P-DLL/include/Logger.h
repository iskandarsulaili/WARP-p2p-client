#pragma once

#include "Types.h"
#include <string>
#include <memory>

namespace P2P {

/**
 * Logger
 * 
 * Provides logging functionality using spdlog.
 * Singleton pattern for global access.
 */
class Logger {
public:
    /**
     * Get singleton instance
     */
    static Logger& GetInstance();

    /**
     * Initialize logger with configuration
     * 
     * @param config Logging configuration
     * @return true if initialized successfully, false otherwise
     */
    bool Initialize(const LoggingConfig& config);

    /**
     * Shutdown logger
     */
    void Shutdown();

    /**
     * Log trace message
     */
    void Trace(const std::string& message);

    /**
     * Log debug message
     */
    void Debug(const std::string& message);

    /**
     * Log info message
     */
    void Info(const std::string& message);

    /**
     * Log warning message
     */
    void Warn(const std::string& message);

    /**
     * Log error message
     */
    void Error(const std::string& message);

    /**
     * Log fatal message
     */
    void Fatal(const std::string& message);

private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    class Impl;
    std::unique_ptr<Impl> impl_;
};

// Convenience macros
#define LOG_TRACE(msg) P2P::Logger::GetInstance().Trace(msg)
#define LOG_DEBUG(msg) P2P::Logger::GetInstance().Debug(msg)
#define LOG_INFO(msg) P2P::Logger::GetInstance().Info(msg)
#define LOG_WARN(msg) P2P::Logger::GetInstance().Warn(msg)
#define LOG_ERROR(msg) P2P::Logger::GetInstance().Error(msg)
#define LOG_FATAL(msg) P2P::Logger::GetInstance().Fatal(msg)

} // namespace P2P

