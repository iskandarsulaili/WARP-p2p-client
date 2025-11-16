#include "../../include/AuthManager.h"
#include "../../include/HttpClient.h"
#include "../../include/Logger.h"
#include <nlohmann/json.hpp>
#include <thread>
#include <atomic>
#include <mutex>
#include <sstream>
#include <iomanip>
#include <ctime>

using json = nlohmann::json;

namespace P2P {

struct AuthManager::Impl {
    std::shared_ptr<HttpClient> http_client;
    std::string coordinator_url;
    std::string current_token;
    std::string peer_id;
    std::chrono::system_clock::time_point token_expiration;
    
    std::atomic<bool> auto_refresh_running{false};
    std::thread auto_refresh_thread;
    std::mutex token_mutex;
    int refresh_interval_seconds = 3600;
};

AuthManager::AuthManager() : impl_(std::make_unique<Impl>()) {
    LOG_DEBUG("AuthManager created");
}

AuthManager::~AuthManager() {
    Shutdown();
    LOG_DEBUG("AuthManager destroyed");
}

bool AuthManager::Initialize(std::shared_ptr<HttpClient> http_client, const std::string& coordinator_url) {
    if (!http_client) {
        LOG_ERROR("AuthManager::Initialize - Invalid HTTP client");
        return false;
    }
    
    impl_->http_client = http_client;
    impl_->coordinator_url = coordinator_url;
    
    LOG_INFO("AuthManager initialized with coordinator: " + coordinator_url);
    return true;
}

void AuthManager::Shutdown() {
    StopAutoRefresh();
    
    std::lock_guard<std::mutex> lock(impl_->token_mutex);
    impl_->current_token.clear();
    impl_->peer_id.clear();
    
    LOG_INFO("AuthManager shutdown complete");
}

void AuthManager::Authenticate(const std::string& peer_id, AuthCallback callback) {
    if (!impl_->http_client) {
        LOG_ERROR("AuthManager not initialized");
        if (callback) callback(false, "AuthManager not initialized");
        return;
    }
    
    LOG_INFO("Authenticating peer: " + peer_id);
    
    // Prepare authentication request
    json auth_request = {
        {"peer_id", peer_id},
        {"client_version", "1.0.0"},
        {"timestamp", std::time(nullptr)}
    };
    
    // Send authentication request
    auto response = impl_->http_client->Post("/api/v1/auth/token", auth_request.dump());
    
    if (!response.success) {
        LOG_ERROR("Authentication failed: " + response.error_message);
        if (callback) callback(false, response.error_message);
        return;
    }
    
    // Parse response
    try {
        auto response_json = json::parse(response.body);
        
        if (!response_json.contains("token")) {
            LOG_ERROR("Authentication response missing token");
            if (callback) callback(false, "Invalid response: missing token");
            return;
        }
        
        std::string token = response_json["token"];
        
        // Store token
        {
            std::lock_guard<std::mutex> lock(impl_->token_mutex);
            impl_->current_token = token;
            impl_->peer_id = peer_id;
            impl_->token_expiration = ParseTokenExpiration(token);
        }
        
        // Update HTTP client with new token
        impl_->http_client->SetAuthToken(token);
        
        LOG_INFO("Authentication successful for peer: " + peer_id);
        if (callback) callback(true, "");
        
    } catch (const json::exception& e) {
        LOG_ERROR("Failed to parse authentication response: " + std::string(e.what()));
        if (callback) callback(false, "Failed to parse response");
    }
}

void AuthManager::RefreshToken(RefreshCallback callback) {
    if (!impl_->http_client) {
        LOG_ERROR("AuthManager not initialized");
        if (callback) callback(false, "");
        return;
    }
    
    std::string current_token;
    {
        std::lock_guard<std::mutex> lock(impl_->token_mutex);
        current_token = impl_->current_token;
    }
    
    if (current_token.empty()) {
        LOG_ERROR("No token to refresh");
        if (callback) callback(false, "");
        return;
    }
    
    LOG_INFO("Refreshing JWT token");
    
    // Send refresh request
    auto response = impl_->http_client->Post("/api/v1/auth/refresh", "{}");
    
    if (!response.success) {
        LOG_ERROR("Token refresh failed: " + response.error_message);
        if (callback) callback(false, "");
        return;
    }
    
    // Parse response
    try {
        auto response_json = json::parse(response.body);
        
        if (!response_json.contains("token")) {
            LOG_ERROR("Refresh response missing token");
            if (callback) callback(false, "");
            return;
        }
        
        std::string new_token = response_json["token"];
        
        // Store new token
        {
            std::lock_guard<std::mutex> lock(impl_->token_mutex);
            impl_->current_token = new_token;
            impl_->token_expiration = ParseTokenExpiration(new_token);
        }
        
        // Update HTTP client
        impl_->http_client->SetAuthToken(new_token);
        
        LOG_INFO("Token refresh successful");
        if (callback) callback(true, new_token);

    } catch (const json::exception& e) {
        LOG_ERROR("Failed to parse refresh response: " + std::string(e.what()));
        if (callback) callback(false, "");
    }
}

std::string AuthManager::GetToken() const {
    std::lock_guard<std::mutex> lock(impl_->token_mutex);
    return impl_->current_token;
}

bool AuthManager::IsAuthenticated() const {
    std::lock_guard<std::mutex> lock(impl_->token_mutex);

    if (impl_->current_token.empty()) {
        return false;
    }

    // Check if token is expired
    auto now = std::chrono::system_clock::now();
    return now < impl_->token_expiration;
}

bool AuthManager::NeedsRefresh() const {
    std::lock_guard<std::mutex> lock(impl_->token_mutex);

    if (impl_->current_token.empty()) {
        return false;
    }

    // Refresh if token expires in less than 5 minutes
    auto now = std::chrono::system_clock::now();
    auto time_until_expiry = std::chrono::duration_cast<std::chrono::minutes>(
        impl_->token_expiration - now
    );

    return time_until_expiry.count() < 5;
}

std::chrono::system_clock::time_point AuthManager::GetTokenExpiration() const {
    std::lock_guard<std::mutex> lock(impl_->token_mutex);
    return impl_->token_expiration;
}

void AuthManager::StartAutoRefresh(int refresh_interval_seconds) {
    if (impl_->auto_refresh_running) {
        LOG_WARN("Auto-refresh already running");
        return;
    }

    impl_->refresh_interval_seconds = refresh_interval_seconds;
    impl_->auto_refresh_running = true;
    impl_->auto_refresh_thread = std::thread(&AuthManager::AutoRefreshWorker, this);

    LOG_INFO("Auto-refresh started (interval: " + std::to_string(refresh_interval_seconds) + "s)");
}

void AuthManager::StopAutoRefresh() {
    if (!impl_->auto_refresh_running) {
        return;
    }

    impl_->auto_refresh_running = false;

    if (impl_->auto_refresh_thread.joinable()) {
        impl_->auto_refresh_thread.join();
    }

    LOG_INFO("Auto-refresh stopped");
}

std::chrono::system_clock::time_point AuthManager::ParseTokenExpiration(const std::string& token) {
    // JWT format: header.payload.signature
    // Payload is base64-encoded JSON containing "exp" field (seconds since epoch)
    try {
        size_t first_dot = token.find('.');
        size_t second_dot = token.find('.', first_dot + 1);
        if (first_dot == std::string::npos || second_dot == std::string::npos) {
            throw std::runtime_error("Invalid JWT format");
        }
        std::string payload_b64 = token.substr(first_dot + 1, second_dot - first_dot - 1);
        // Pad base64 if needed
        while (payload_b64.size() % 4 != 0) payload_b64 += '=';
        // Replace URL-safe chars
        std::replace(payload_b64.begin(), payload_b64.end(), '-', '+');
        std::replace(payload_b64.begin(), payload_b64.end(), '_', '/');
        // Decode base64
        std::string payload_json;
        {
            // Use a simple base64 decoder (C++17)
            static const std::string base64_chars =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            auto decode = [](const std::string& in) -> std::string {
                std::string out;
                std::vector<int> T(256, -1);
                for (int i = 0; i < 64; i++) T[base64_chars[i]] = i;
                int val = 0, valb = -8;
                for (unsigned char c : in) {
                    if (T[c] == -1) break;
                    val = (val << 6) + T[c];
                    valb += 6;
                    if (valb >= 0) {
                        out.push_back(char((val >> valb) & 0xFF));
                        valb -= 8;
                    }
                }
                return out;
            };
            payload_json = decode(payload_b64);
        }
        auto payload = json::parse(payload_json);
        if (!payload.contains("exp")) {
            throw std::runtime_error("JWT payload missing 'exp'");
        }
        int64_t exp = payload["exp"];
        std::chrono::system_clock::time_point tp{std::chrono::seconds(exp)};
        return tp;
    } catch (const std::exception& e) {
        LOG_WARN(std::string("Failed to parse JWT expiration: ") + e.what() + ", defaulting to 24h");
        auto now = std::chrono::system_clock::now();
        return now + std::chrono::hours(24);
    }
}

void AuthManager::AutoRefreshWorker() {
    LOG_DEBUG("Auto-refresh worker started");

    while (impl_->auto_refresh_running) {
        // Sleep for refresh interval
        for (int i = 0; i < impl_->refresh_interval_seconds && impl_->auto_refresh_running; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (!impl_->auto_refresh_running) {
            break;
        }

        // Check if refresh is needed
        if (NeedsRefresh()) {
            LOG_INFO("Auto-refresh triggered");
            RefreshToken([](bool success, const std::string& /* new_token */) {
                if (success) {
                    LOG_INFO("Auto-refresh successful");
                } else {
                    LOG_ERROR("Auto-refresh failed");
                }
            });
        }
    }

    LOG_DEBUG("Auto-refresh worker stopped");
}

} // namespace P2P

