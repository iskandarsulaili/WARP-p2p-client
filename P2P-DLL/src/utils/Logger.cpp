#include "Logger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>

namespace P2P {

class Logger::Impl {
public:
    std::shared_ptr<spdlog::logger> logger;
    bool debug_enabled = false;
    std::string correlation_id;
};

Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}

bool Logger::Initialize(const LoggingConfig& config) {
    try {
        impl_ = std::make_unique<Impl>();

        std::vector<spdlog::sink_ptr> sinks;

        // File sink
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            config.file,
            config.max_file_size_mb * 1024 * 1024,
            config.max_files
        );
        sinks.push_back(file_sink);

        // Console sink (if enabled)
        if (config.console_output) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            sinks.push_back(console_sink);
        }

        // Create logger
        if (config.async_logging) {
            spdlog::init_thread_pool(8192, 1);
            impl_->logger = std::make_shared<spdlog::async_logger>(
                "p2p_dll",
                sinks.begin(),
                sinks.end(),
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::block
            );
        } else {
            impl_->logger = std::make_shared<spdlog::logger>("p2p_dll", sinks.begin(), sinks.end());
        }

        // Set log level
        if (config.level == "trace") {
            impl_->logger->set_level(spdlog::level::trace);
        } else if (config.level == "debug") {
            impl_->logger->set_level(spdlog::level::debug);
        } else if (config.level == "info") {
            impl_->logger->set_level(spdlog::level::info);
        } else if (config.level == "warn") {
            impl_->logger->set_level(spdlog::level::warn);
        } else if (config.level == "error") {
            impl_->logger->set_level(spdlog::level::err);
        } else if (config.level == "fatal") {
            impl_->logger->set_level(spdlog::level::critical);
        }

        // Set pattern
        impl_->logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");

        // Register as default logger
        spdlog::set_default_logger(impl_->logger);

        impl_->logger->info("Logger initialized");
        return true;
    }
    catch (const std::exception&) {
        // Exception caught during logger initialization
        return false;
    }
}

void Logger::Shutdown() {
    if (impl_ && impl_->logger) {
        impl_->logger->info("Logger shutting down");
        impl_->logger->flush();
        spdlog::shutdown();
    }
}

void Logger::Trace(const std::string& message, const std::string& correlation_id) {
    if (impl_ && impl_->logger) {
        impl_->logger->trace("[CID:{}] {}", correlation_id, message);
    }
}

void Logger::Debug(const std::string& message, const std::string& correlation_id) {
    if (impl_ && impl_->logger && impl_->debug_enabled) {
        impl_->logger->debug("[CID:{}] {}", correlation_id, message);
    }
}

void Logger::Info(const std::string& message, const std::string& correlation_id) {
    if (impl_ && impl_->logger) {
        impl_->logger->info("[CID:{}] {}", correlation_id, message);
    }
}

void Logger::Warn(const std::string& message, const std::string& correlation_id) {
    if (impl_ && impl_->logger) {
        impl_->logger->warn("[CID:{}] {}", correlation_id, message);
    }
}

void Logger::Error(const std::string& message, const std::string& correlation_id) {
    if (impl_ && impl_->logger) {
        impl_->logger->error("[CID:{}] {}", correlation_id, message);
    }
}

void Logger::Fatal(const std::string& message, const std::string& correlation_id) {
    if (impl_ && impl_->logger) {
        impl_->logger->critical("[CID:{}] {}", correlation_id, message);
    }
}

void Logger::SetDebugEnabled(bool enabled) {
    if (impl_) impl_->debug_enabled = enabled;
}
bool Logger::IsDebugEnabled() const {
    return impl_ ? impl_->debug_enabled : false;
}
void Logger::SetCorrelationId(const std::string& id) {
    if (impl_) impl_->correlation_id = id;
}
std::string Logger::GetCorrelationId() const {
    return impl_ ? impl_->correlation_id : "";
}

} // namespace P2P

