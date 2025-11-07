#include "../../include/SignalingClient.h"
#include "../../include/Logger.h"
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

using json = nlohmann::json;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

namespace P2P {

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

struct SignalingClient::Impl {
    client ws_client;
    websocketpp::connection_hdl connection_handle;
    std::string server_url;
    
    std::atomic<bool> connected{false};
    std::atomic<bool> should_reconnect{true};
    std::atomic<bool> running{false};
    
    std::thread io_thread;
    std::mutex callback_mutex;
    
    OnMessageCallback on_message;
    OnConnectedCallback on_connected;
    OnDisconnectedCallback on_disconnected;
    
    int reconnect_delay_ms = 1000;
    int max_reconnect_delay_ms = 30000;
    int current_reconnect_delay_ms = 1000;
};

SignalingClient::SignalingClient() : impl_(std::make_unique<Impl>()) {
    LOG_DEBUG("SignalingClient created");
    
    impl_->ws_client.clear_access_channels(websocketpp::log::alevel::all);
    impl_->ws_client.clear_error_channels(websocketpp::log::elevel::all);
    
    impl_->ws_client.init_asio();
    impl_->ws_client.start_perpetual();
}

SignalingClient::~SignalingClient() {
    Disconnect();
    LOG_DEBUG("SignalingClient destroyed");
}

bool SignalingClient::Connect(const std::string& url) {
    if (impl_->connected) {
        LOG_WARN("Already connected");
        return true;
    }
    
    LOG_INFO("Connecting to: " + url);
    impl_->server_url = url;
    impl_->should_reconnect = true;
    
    try {
        websocketpp::lib::error_code ec;
        client::connection_ptr con = impl_->ws_client.get_connection(url, ec);
        
        if (ec) {
            LOG_ERROR("Connection failed: " + ec.message());
            return false;
        }
        
        impl_->connection_handle = con->get_handle();
        impl_->ws_client.connect(con);
        
        if (!impl_->running) {
            impl_->running = true;
            impl_->io_thread = std::thread([this]() {
                impl_->ws_client.run();
            });
        }
        
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
    
    if (impl_->connected) {
        try {
            impl_->ws_client.close(impl_->connection_handle, 
                                  websocketpp::close::status::normal, 
                                  "Client disconnect");
        } catch (const std::exception& e) {
            LOG_ERROR("Close error: " + std::string(e.what()));
        }
    }
    
    impl_->ws_client.stop_perpetual();
    
    if (impl_->io_thread.joinable()) {
        impl_->io_thread.join();
    }
    
    impl_->connected = false;
}

bool SignalingClient::IsConnected() const {
    return impl_->connected;
}

void SignalingClient::SendMessage(const std::string& message) {
    if (!impl_->connected) {
        LOG_ERROR("Not connected");
        return;
    }
    
    try {
        impl_->ws_client.send(impl_->connection_handle, message, 
                             websocketpp::frame::opcode::text);
    } catch (const std::exception& e) {
        LOG_ERROR("Send error: " + std::string(e.what()));
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
