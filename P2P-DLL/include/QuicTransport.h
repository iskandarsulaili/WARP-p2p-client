#pragma once

#include "ITransport.h"
#include <string>
#include <vector>
#include <functional>

namespace P2P {

/**
 * QuicTransport - Stub for QUIC protocol transport.
 * To be implemented with a QUIC library (e.g., msquic, quiche, etc.)
 */
class QuicTransport : public ITransport {
public:
    QuicTransport();
    ~QuicTransport() override;

    bool Connect(const std::string& address, uint16_t port) override;
    void Disconnect() override;
    bool SendData(const void* data, size_t size) override;
    void SetOnReceive(std::function<void(const std::vector<uint8_t>&)> callback) override;
    bool IsConnected() const override;

private:
    // Internal state for QUIC connection (stub)
    bool connected_;
    std::function<void(const std::vector<uint8_t>&)> on_receive_;
};

} // namespace P2P