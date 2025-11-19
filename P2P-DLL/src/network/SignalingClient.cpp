#include "../../include/SignalingClient.h"
#include "../../include/Logger.h"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <nlohmann/json.hpp>
// Compatibility: ensure correct namespace and API usage for latest Boost/Beast and nlohmann/json
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

#ifdef SendMessage
#undef SendMessage
#endif

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;
using json = nlohmann::json;

namespace P2P {

class SignalingClient::Impl {
public:
    net::io_context ioc;
    ssl::context ssl_ctx{ssl::context::tlsv12_client};
    std::unique_ptr<websocket::stream<ssl::stream<tcp::socket>>> ws;

    std::string server_url;
    std::string peer_id;
    std::string session_id;
    std::string host;
    std::string port;
    std::string path;

    std::atomic<bool> connected{false};
    std::atomic<bool> should_reconnect{true};
    std::atomic<bool> running{false};

    std::thread io_thread;
    std::mutex callback_mutex;
    beast::flat_buffer buffer;

    OnMessageCallback on_message;
    OnConnectedCallback on_connected;
    OnDisconnectedCallback on_disconnected;

    int reconnect_delay_ms = 1000;
    int max_reconnect_delay_ms = 30000;
    int current_reconnect_delay_ms = 1000;
    int max_retries = 10;
    int retry_count = 0;

    Impl() {
        ssl_ctx.set_verify_mode(ssl::verify_peer);
        ssl_ctx.set_default_verify_paths();
    }
};

SignalingClient::SignalingClient() : impl_(std::make_unique<Impl>()) {
    LOG_DEBUG("SignalingClient created");
}

SignalingClient::~SignalingClient() {
    Disconnect();
    LOG_DEBUG("SignalingClient destroyed");
}

bool SignalingClient::Connect(const std::string& url, const std::string& peer_id, const std::string& session_id) {
    if (impl_->connected) {
        LOG_WARN("Already connected");
        return true;
    }

    LOG_INFO("Connecting to: " + url);
    impl_->server_url = url;
    impl_->peer_id = peer_id;
    impl_->session_id = session_id;
    impl_->should_reconnect = true;
    impl_->retry_count = 0;
    impl_->current_reconnect_delay_ms = impl_->reconnect_delay_ms;

    try {
        // Parse WebSocket URL (wss://host:port/path)
        std::string url_copy = url;
        bool use_ssl = (url_copy.find("wss://") == 0);

        if (use_ssl) {
            url_copy = url_copy.substr(6); // Remove "wss://"
        } else if (url_copy.find("ws://") == 0) {
            url_copy = url_copy.substr(5); // Remove "ws://"
        }

        // Extract host, port, and path
        size_t slash_pos = url_copy.find('/');
        std::string host_port = (slash_pos != std::string::npos) ? url_copy.substr(0, slash_pos) : url_copy;
        impl_->path = (slash_pos != std::string::npos) ? url_copy.substr(slash_pos) : "/";
        size_t colon_pos = host_port.find(':');
        if (colon_pos != std::string::npos) {
            impl_->host = host_port.substr(0, colon_pos);
            impl_->port = host_port.substr(colon_pos + 1);
        } else {
            impl_->host = host_port;
            impl_->port = use_ssl ? "443" : "80";
        }

        // Start connection in background thread
        if (!impl_->running) {
            impl_->running = true;
            impl_->io_thread = std::thread([this]() {
                while (impl_->should_reconnect && impl_->retry_count < impl_->max_retries) {
                    try {
                        LOG_INFO("SignalingClient: Attempting connection (try " + std::to_string(impl_->retry_count + 1) + ")");
                        // Resolve hostname
                        tcp::resolver resolver(impl_->ioc);
                        auto const results = resolver.resolve(impl_->host, impl_->port);

                        // Create WebSocket stream
                        impl_->ws = std::make_unique<websocket::stream<ssl::stream<tcp::socket>>>(
                            impl_->ioc, impl_->ssl_ctx);

                        // Connect to server
                        auto ep = net::connect(beast::get_lowest_layer(*impl_->ws), results);

                        // Perform SSL handshake
                        impl_->ws->next_layer().handshake(ssl::stream_base::client);

                        // Set WebSocket options
                        impl_->ws->set_option(websocket::stream_base::decorator(
                            [](websocket::request_type& req) {
                                req.set(http::field::user_agent, "P2P-Network-Client/1.0");
                            }));

                        // Perform WebSocket handshake
                        impl_->ws->handshake(impl_->host, impl_->path);

                        impl_->connected = true;
                        impl_->retry_count = 0;
                        LOG_INFO("Connected to: " + impl_->server_url);

                        // Call connected callback
                        {
                            std::lock_guard<std::mutex> lock(impl_->callback_mutex);
                            if (impl_->on_connected) {
                                impl_->on_connected();
                            }
                        }

                        // Start read loop
                        while (impl_->running && impl_->connected) {
                            try {
                                impl_->buffer.clear();
                                impl_->ws->read(impl_->buffer);

                                std::string message = beast::buffers_to_string(impl_->buffer.data());

                                // Call message callback
                                std::lock_guard<std::mutex> lock(impl_->callback_mutex);
                                if (impl_->on_message) {
                                    impl_->on_message(message);
                                }
                            } catch (const beast::system_error& se) {
                                if (se.code() != websocket::error::closed) {
                                    LOG_ERROR("Read error: " + std::string(se.what()));
                                }
                                break;
                            }
                        }

                        impl_->connected = false;

                        // Call disconnected callback
                        {
                            std::lock_guard<std::mutex> lock(impl_->callback_mutex);
                            if (impl_->on_disconnected) {
                                impl_->on_disconnected();
                            }
                        }

                        // If we should reconnect, wait and try again
                        if (impl_->should_reconnect) {
                            impl_->retry_count++;
                            LOG_WARN("SignalingClient: Disconnected, will retry in " +
                                     std::to_string(impl_->current_reconnect_delay_ms) + " ms (attempt " +
                                     std::to_string(impl_->retry_count) + "/" + std::to_string(impl_->max_retries) + ")");
                            std::this_thread::sleep_for(std::chrono::milliseconds(impl_->current_reconnect_delay_ms));
                            impl_->current_reconnect_delay_ms = std::min(impl_->current_reconnect_delay_ms * 2, impl_->max_reconnect_delay_ms);
                        }
                    } catch (const std::exception& e) {
                        LOG_ERROR("Connection thread error: " + std::string(e.what()));
                        impl_->connected = false;
                        impl_->retry_count++;
                        LOG_WARN("SignalingClient: Connection failed, will retry in " +
                                 std::to_string(impl_->current_reconnect_delay_ms) + " ms (attempt " +
                                 std::to_string(impl_->retry_count) + "/" + std::to_string(impl_->max_retries) + ")");
                        std::this_thread::sleep_for(std::chrono::milliseconds(impl_->current_reconnect_delay_ms));
                        impl_->current_reconnect_delay_ms = std::min(impl_->current_reconnect_delay_ms * 2, impl_->max_reconnect_delay_ms);
                    }
                }
                // Exceeded max retries
                if (impl_->retry_count >= impl_->max_retries) {
                    LOG_ERROR("SignalingClient: Exceeded maximum reconnection attempts (" +
                              std::to_string(impl_->max_retries) + "). Giving up.");
                    impl_->should_reconnect = false;
                    impl_->connected = false;
                    // Call disconnected callback one last time
                    std::lock_guard<std::mutex> lock(impl_->callback_mutex);
                    if (impl_->on_disconnected) {
                        impl_->on_disconnected();
                    }
                }
            });
        }

        // Wait a bit for connection to establish
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Connection exception: " + std::string(e.what()));
        return false;
    }
}

void SignalingClient::Disconnect() {
    LOG_INFO("Disconnecting");

    impl_->should_reconnect = false;
    impl_->running = false;

    if (impl_->connected && impl_->ws) {
        try {
            impl_->ws->close(websocket::close_code::normal);
        } catch (const std::exception& e) {
            LOG_ERROR("Close error: " + std::string(e.what()));
        }
    }

    impl_->ioc.stop();

    if (impl_->io_thread.joinable()) {
        impl_->io_thread.join();
    }

    impl_->connected = false;
    impl_->ws.reset();
}

bool SignalingClient::IsConnected() const {
    return impl_->connected;
}

bool SignalingClient::SendMessage(const std::string& message) {
    if (!impl_->connected || !impl_->ws) {
        LOG_ERROR("Not connected");
        return false;
    }

    try {
        impl_->ws->write(net::buffer(message));
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Send error: " + std::string(e.what()));
        return false;
    }
}

void SignalingClient::SetOnMessageCallback(OnMessageCallback callback) {
    std::lock_guard<std::mutex> lock(impl_->callback_mutex);
    impl_->on_message = callback;
}

void SignalingClient::SetOnConnectedCallback(OnConnectedCallback callback) {
    std::lock_guard<std::mutex> lock(impl_->callback_mutex);
    impl_->on_connected = callback;
}

void SignalingClient::SetOnDisconnectedCallback(OnDisconnectedCallback callback) {
    std::lock_guard<std::mutex> lock(impl_->callback_mutex);
    impl_->on_disconnected = callback;
}

} // namespace P2P