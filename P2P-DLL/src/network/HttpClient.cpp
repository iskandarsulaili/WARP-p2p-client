#include "HttpClient.h"
#include "Logger.h"
#include <httplib.h>
#include <sstream>

namespace P2P {

class HttpClient::Impl {
public:
    std::string base_url;
    std::string auth_token;
    int timeout_seconds = 30;
    std::unique_ptr<httplib::Client> client;
    
    void UpdateClient() {
        if (!base_url.empty()) {
            client = std::make_unique<httplib::Client>(base_url);
            client->set_connection_timeout(timeout_seconds);
            client->set_read_timeout(timeout_seconds);
            client->set_write_timeout(timeout_seconds);
        }
    }
};

HttpClient::HttpClient() : impl_(std::make_unique<Impl>()) {
    LOG_DEBUG("HttpClient created");
}

HttpClient::~HttpClient() {
    LOG_DEBUG("HttpClient destroyed");
}

void HttpClient::SetBaseUrl(const std::string& base_url) {
    impl_->base_url = base_url;
    impl_->UpdateClient();
    LOG_INFO("HttpClient base URL set to: " + base_url);
}

void HttpClient::SetAuthToken(const std::string& token) {
    impl_->auth_token = token;
    LOG_DEBUG("HttpClient auth token updated");
}

void HttpClient::SetTimeout(int timeout_seconds) {
    impl_->timeout_seconds = timeout_seconds;
    if (impl_->client) {
        impl_->client->set_connection_timeout(timeout_seconds);
        impl_->client->set_read_timeout(timeout_seconds);
        impl_->client->set_write_timeout(timeout_seconds);
    }
    LOG_DEBUG("HttpClient timeout set to: " + std::to_string(timeout_seconds) + "s");
}

HttpResponse HttpClient::Get(const std::string& path, const std::map<std::string, std::string>& query_params) {
    HttpResponse response;
    
    if (!impl_->client) {
        LOG_ERROR("HttpClient not configured");
        response.success = false;
        response.error_message = "Client not configured";
        return response;
    }
    
    // Build query string
    std::string full_path = path;
    if (!query_params.empty()) {
        full_path += "?";
        bool first = true;
        for (const auto& [key, value] : query_params) {
            if (!first) full_path += "&";
            full_path += key + "=" + value;
            first = false;
        }
    }
    
    LOG_DEBUG("GET " + full_path);
    
    auto headers = BuildHeaders();
    httplib::Headers httplib_headers;
    for (const auto& [key, value] : headers) {
        httplib_headers.insert({key, value});
    }
    
    auto result = impl_->client->Get(full_path, httplib_headers);
    
    if (result) {
        response.status_code = result->status;
        response.body = result->body;
        response.success = (result->status >= 200 && result->status < 300);
        
        for (const auto& [key, value] : result->headers) {
            response.headers[key] = value;
        }
        
        LOG_DEBUG("GET " + full_path + " -> " + std::to_string(result->status));
    } else {
        response.success = false;
        response.error_message = "Request failed: " + httplib::to_string(result.error());
        LOG_ERROR("GET " + full_path + " failed: " + response.error_message);
    }
    
    return response;
}

HttpResponse HttpClient::Post(const std::string& path, const std::string& body) {
    HttpResponse response;
    
    if (!impl_->client) {
        LOG_ERROR("HttpClient not configured");
        response.success = false;
        response.error_message = "Client not configured";
        return response;
    }
    
    LOG_DEBUG("POST " + path);
    
    auto headers = BuildHeaders();
    headers["Content-Type"] = "application/json";
    
    httplib::Headers httplib_headers;
    for (const auto& [key, value] : headers) {
        httplib_headers.insert({key, value});
    }
    
    auto result = impl_->client->Post(path, httplib_headers, body, "application/json");
    
    if (result) {
        response.status_code = result->status;
        response.body = result->body;
        response.success = (result->status >= 200 && result->status < 300);
        
        for (const auto& [key, value] : result->headers) {
            response.headers[key] = value;
        }
        
        LOG_DEBUG("POST " + path + " -> " + std::to_string(result->status));
    } else {
        response.success = false;
        response.error_message = "Request failed: " + httplib::to_string(result.error());
        LOG_ERROR("POST " + path + " failed: " + response.error_message);
    }
    
    return response;
}

HttpResponse HttpClient::Put(const std::string& path, const std::string& body) {
    HttpResponse response;
    
    if (!impl_->client) {
        LOG_ERROR("HttpClient not configured");
        response.success = false;
        response.error_message = "Client not configured";
        return response;
    }
    
    LOG_DEBUG("PUT " + path);
    
    auto headers = BuildHeaders();
    headers["Content-Type"] = "application/json";
    
    httplib::Headers httplib_headers;
    for (const auto& [key, value] : headers) {
        httplib_headers.insert({key, value});
    }
    
    auto result = impl_->client->Put(path, httplib_headers, body, "application/json");
    
    if (result) {
        response.status_code = result->status;
        response.body = result->body;
        response.success = (result->status >= 200 && result->status < 300);
        
        for (const auto& [key, value] : result->headers) {
            response.headers[key] = value;
        }
        
        LOG_DEBUG("PUT " + path + " -> " + std::to_string(result->status));
    } else {
        response.success = false;
        response.error_message = "Request failed: " + httplib::to_string(result.error());
        LOG_ERROR("PUT " + path + " failed: " + response.error_message);
    }
    
    return response;
}

HttpResponse HttpClient::Delete(const std::string& path) {
    HttpResponse response;

    if (!impl_->client) {
        LOG_ERROR("HttpClient not configured");
        response.success = false;
        response.error_message = "Client not configured";
        return response;
    }

    LOG_DEBUG("DELETE " + path);

    auto headers = BuildHeaders();
    httplib::Headers httplib_headers;
    for (const auto& [key, value] : headers) {
        httplib_headers.insert({key, value});
    }

    auto result = impl_->client->Delete(path, httplib_headers);

    if (result) {
        response.status_code = result->status;
        response.body = result->body;
        response.success = (result->status >= 200 && result->status < 300);

        for (const auto& [key, value] : result->headers) {
            response.headers[key] = value;
        }

        LOG_DEBUG("DELETE " + path + " -> " + std::to_string(result->status));
    } else {
        response.success = false;
        response.error_message = "Request failed: " + httplib::to_string(result.error());
        LOG_ERROR("DELETE " + path + " failed: " + response.error_message);
    }

    return response;
}

HttpResponse HttpClient::SendRequest(const HttpRequest& request) {
    HttpResponse response;

    if (!impl_->client) {
        LOG_ERROR("HttpClient not configured");
        response.success = false;
        response.error_message = "Client not configured";
        return response;
    }

    LOG_DEBUG(request.method + " " + request.url);

    // Build headers
    httplib::Headers httplib_headers;
    for (const auto& [key, value] : request.headers) {
        httplib_headers.insert({key, value});
    }

    // Add default headers if not present
    auto default_headers = BuildHeaders();
    for (const auto& [key, value] : default_headers) {
        if (request.headers.find(key) == request.headers.end()) {
            httplib_headers.insert({key, value});
        }
    }

    // Parse URL to extract path
    std::string path = request.url;
    if (path.find(impl_->base_url) == 0) {
        path = path.substr(impl_->base_url.length());
    }

    // Execute request based on method
    httplib::Result result;
    if (request.method == "GET") {
        result = impl_->client->Get(path, httplib_headers);
    } else if (request.method == "POST") {
        result = impl_->client->Post(path, httplib_headers, request.body, "application/json");
    } else if (request.method == "PUT") {
        result = impl_->client->Put(path, httplib_headers, request.body, "application/json");
    } else if (request.method == "DELETE") {
        result = impl_->client->Delete(path, httplib_headers);
    } else {
        response.success = false;
        response.error_message = "Unsupported HTTP method: " + request.method;
        LOG_ERROR(response.error_message);
        return response;
    }

    if (result) {
        response.status_code = result->status;
        response.body = result->body;
        response.success = (result->status >= 200 && result->status < 300);

        for (const auto& [key, value] : result->headers) {
            response.headers[key] = value;
        }

        LOG_DEBUG(request.method + " " + path + " -> " + std::to_string(result->status));
    } else {
        response.success = false;
        response.error_message = "Request failed: " + httplib::to_string(result.error());
        LOG_ERROR(request.method + " " + path + " failed: " + response.error_message);
    }

    return response;
}

bool HttpClient::IsConfigured() const {
    return impl_->client != nullptr && !impl_->base_url.empty();
}

std::string HttpClient::BuildUrl(const std::string& path) const {
    return impl_->base_url + path;
}

std::map<std::string, std::string> HttpClient::BuildHeaders() const {
    std::map<std::string, std::string> headers;

    if (!impl_->auth_token.empty()) {
        headers["Authorization"] = "Bearer " + impl_->auth_token;
    }

    headers["User-Agent"] = "P2P-DLL/1.0.0";
    headers["Accept"] = "application/json";

    return headers;
}

} // namespace P2P
