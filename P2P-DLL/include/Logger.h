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
    void Trace(const std::string& message, const std::string& correlation_id = "");
    void Debug(const std::string& message, const std::string& correlation_id = "");
    void Info(const std::string& message, const std::string& correlation_id = "");
    void Warn(const std::string& message, const std::string& correlation_id = "");
    void Error(const std::string& message, const std::string& correlation_id = "");
    void Fatal(const std::string& message, const std::string& correlation_id = "");

    // Runtime debug toggle
    void SetDebugEnabled(bool enabled);
    bool IsDebugEnabled() const;

    // Set correlation ID for context
    void SetCorrelationId(const std::string& id);
    std::string GetCorrelationId() const;

private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    class Impl;
    std::unique_ptr<Impl> impl_;
};

#define LOG_TRACE(msg) P2P::Logger::GetInstance().Trace(msg, P2P::Logger::GetInstance().GetCorrelationId())
#define LOG_DEBUG(msg) P2P::Logger::GetInstance().Debug(msg, P2P::Logger::GetInstance().GetCorrelationId())
#define LOG_INFO(msg) P2P::Logger::GetInstance().Info(msg, P2P::Logger::GetInstance().GetCorrelationId())
#define LOG_WARN(msg) P2P::Logger::GetInstance().Warn(msg, P2P::Logger::GetInstance().GetCorrelationId())
#define LOG_ERROR(msg) P2P::Logger::GetInstance().Error(msg, P2P::Logger::GetInstance().GetCorrelationId())
#define LOG_FATAL(msg) P2P::Logger::GetInstance().Fatal(msg, P2P::Logger::GetInstance().GetCorrelationId())

} // namespace P2P

