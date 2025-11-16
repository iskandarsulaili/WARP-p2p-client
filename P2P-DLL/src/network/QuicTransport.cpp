#include "../../include/QuicTransport.h"
#include "../../include/Logger.h"

namespace P2P {

QuicTransport::QuicTransport() : connected_(false) {
    LOG_DEBUG("QuicTransport created (stub)");
}

QuicTransport::~QuicTransport() {
    Disconnect();
    LOG_DEBUG("QuicTransport destroyed (stub)");
}

bool QuicTransport::Connect(const std::string& address, uint16_t port) {
    LOG_INFO("QuicTransport::Connect called (stub) - address: " + address + ", port: " + std::to_string(port));
    // TODO: Implement actual QUIC connection logic
    connected_ = true;
    return connected_;
}

void QuicTransport::Disconnect() {
    if (connected_) {
        LOG_INFO("QuicTransport::Disconnect called (stub)");
        // TODO: Implement actual QUIC disconnect logic
        connected_ = false;
    }
}

bool QuicTransport::SendData(const void* data, size_t size) {
    if (!connected_) {
        LOG_ERROR("QuicTransport::SendData failed - not connected (stub)");
        return false;
    }
    LOG_DEBUG("QuicTransport::SendData called (stub) - size: " + std::to_string(size));
    // TODO: Implement actual QUIC send logic
    return true;
}

void QuicTransport::SetOnReceive(std::function<void(const std::vector<uint8_t>&)> callback) {
    on_receive_ = std::move(callback);
    LOG_DEBUG("QuicTransport::SetOnReceive set (stub)");
}

bool QuicTransport::IsConnected() const {
    return connected_;
}

} // namespace P2P