#pragma once

#include <string>
#include <memory>
#include <functional>
#include <chrono>

namespace P2P {

// Forward declarations
class HttpClient;

/**
 * @brief Authentication manager for JWT token handling
 * 
 * Manages JWT token acquisition, refresh, and validation.
 * Integrates with coordinator service for authentication.
 */
class AuthManager {
public:
    /**
     * @brief Authentication result callback
     * @param success True if authentication succeeded
     * @param error_message Error message if failed
     */
    using AuthCallback = std::function<void(bool success, const std::string& error_message)>;
    
    /**
     * @brief Token refresh callback
     * @param success True if refresh succeeded
     * @param new_token New JWT token if succeeded
     */
    using RefreshCallback = std::function<void(bool success, const std::string& new_token)>;
    
    AuthManager();
    ~AuthManager();
    
    // Disable copy
    AuthManager(const AuthManager&) = delete;
    AuthManager& operator=(const AuthManager&) = delete;
    
    /**
     * @brief Initialize authentication manager
     * @param http_client Shared HTTP client instance
     * @param coordinator_url Coordinator service base URL
     * @return True if initialization succeeded
     */
    bool Initialize(std::shared_ptr<HttpClient> http_client, const std::string& coordinator_url);
    
    /**
     * @brief Shutdown authentication manager
     */
    void Shutdown();
    
    /**
     * @brief Authenticate with coordinator service (synchronous)
     * @param peer_id Unique peer identifier
     * @param error_message Output parameter for error message if failed
     * @return True if authentication succeeded
     */
    bool AuthenticateSync(const std::string& peer_id, std::string& error_message);

    /**
     * @brief Authenticate with coordinator service (async with callback)
     * @param peer_id Unique peer identifier
     * @param callback Callback for authentication result
     */
    void Authenticate(const std::string& peer_id, AuthCallback callback);
    
    /**
     * @brief Refresh JWT token
     * @param callback Callback for refresh result
     */
    void RefreshToken(RefreshCallback callback);
    
    /**
     * @brief Get current JWT token
     * @return Current token or empty string if not authenticated
     */
    std::string GetToken() const;
    
    /**
     * @brief Check if currently authenticated
     * @return True if authenticated with valid token
     */
    bool IsAuthenticated() const;
    
    /**
     * @brief Check if token is expired or about to expire
     * @return True if token needs refresh
     */
    bool NeedsRefresh() const;
    
    /**
     * @brief Get token expiration time
     * @return Token expiration timestamp
     */
    std::chrono::system_clock::time_point GetTokenExpiration() const;
    
    /**
     * @brief Start automatic token refresh
     * @param refresh_interval_seconds Interval between refresh checks (default: 3600s = 1 hour)
     */
    void StartAutoRefresh(int refresh_interval_seconds = 3600);
    
    /**
     * @brief Stop automatic token refresh
     */
    void StopAutoRefresh();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    
    /**
     * @brief Parse JWT token to extract expiration time
     * @param token JWT token string
     * @return Expiration timestamp
     */
    std::chrono::system_clock::time_point ParseTokenExpiration(const std::string& token);
    
    /**
     * @brief Auto-refresh worker thread function
     */
    void AutoRefreshWorker();
};

} // namespace P2P

