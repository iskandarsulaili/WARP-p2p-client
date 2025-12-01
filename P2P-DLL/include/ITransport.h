#pragma once

#include <string>
#include <vector>
#include <functional>

namespace P2P {

/**
 * ITransport - Abstract interface for network transport (WebRTC, QUIC, etc.)
 */
class ITransport {
public:
    virtual ~ITransport() = default;

    // Connect to remote peer or server
    virtual bool Connect(const std::string& address, uint16_t port) = 0;

    // Disconnect from remote
    virtual void Disconnect() = 0;

    // Send data
    virtual bool SendData(const void* data, size_t size) = 0;

    // Set receive callback
    virtual void SetOnReceive(std::function<void(const std::vector<uint8_t>&)> callback) = 0;

    // Check if connected
    virtual bool IsConnected() const = 0;
};

} // namespace P2P