#pragma once

#include <string>
#include <map>
#include <memory>
#include <functional>

namespace P2P {

/**
 * HTTP Request
 */
struct HttpRequest {
    std::string method;  // GET, POST, PUT, DELETE
    std::string url;
    std::map<std::string, std::string> headers;
    std::string body;
};

/**
 * HTTP Response
 */
struct HttpResponse {
    int status_code;
    std::string body;
    std::map<std::string, std::string> headers;
    bool success;
    std::string error_message;
};

/**
 * HTTP Client
 * 
 * Wrapper around cpp-httplib for REST API communication with coordinator.
 * Handles authentication, retries, and error handling.
 */
class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    /**
     * Set base URL for API requests
     * 
     * @param base_url Base URL (e.g., "http://localhost:8001/api/v1")
     */
    void SetBaseUrl(const std::string& base_url);

    /**
     * Set authentication token
     * 
     * @param token JWT token
     */
    void SetAuthToken(const std::string& token);

    /**
     * Set timeout
     * 
     * @param timeout_seconds Timeout in seconds
     */
    void SetTimeout(int timeout_seconds);

    /**
     * GET request
     * 
     * @param path API path (e.g., "/hosts/")
     * @param query_params Query parameters
     * @return HTTP response
     */
    HttpResponse Get(const std::string& path, const std::map<std::string, std::string>& query_params = {});

    /**
     * POST request
     * 
     * @param path API path
     * @param body Request body (JSON string)
     * @return HTTP response
     */
    HttpResponse Post(const std::string& path, const std::string& body);

    /**
     * PUT request
     * 
     * @param path API path
     * @param body Request body (JSON string)
     * @return HTTP response
     */
    HttpResponse Put(const std::string& path, const std::string& body);

    /**
     * DELETE request
     *
     * @param path API path
     * @return HTTP response
     */
    HttpResponse Delete(const std::string& path);

    /**
     * Send generic HTTP request
     *
     * @param request HTTP request
     * @return HTTP response
     */
    HttpResponse SendRequest(const HttpRequest& request);

    /**
     * Check if client is configured
     */
    bool IsConfigured() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;

    // Helper to build full URL
    std::string BuildUrl(const std::string& path) const;
    
    // Helper to add auth headers
    std::map<std::string, std::string> BuildHeaders() const;
};

} // namespace P2P

