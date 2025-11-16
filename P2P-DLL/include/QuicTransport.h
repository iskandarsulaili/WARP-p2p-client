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
    // Internal state for QUIC connection
    bool connected_;
    void* quic_session_; // Opaque pointer to QUIC session/context
    std::function<void(const std::vector<uint8_t>&)> on_receive_;
    std::mutex conn_mutex_;
    std::string remote_addr_;
    uint16_t remote_port_;
    // Security: session keys, validation, etc.
    std::vector<uint8_t> session_key_;
    bool validate_and_decrypt(const uint8_t* data, size_t size, std::vector<uint8_t>& out);
    void handle_receive(const uint8_t* data, size_t size);
    void cleanup();
};

} // namespace P2P