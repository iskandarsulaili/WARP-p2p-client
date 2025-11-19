#pragma once

#include "ITransport.h"
#include <string>
#include <vector>
#include <functional>
#include <mutex>

// MsQuic header
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <msquic.h>

namespace P2P {

/**
 * QuicTransport - Production implementation using MsQuic.
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
    // MsQuic Callbacks
    static QUIC_STATUS QUIC_API ClientConnectionCallback(
        _In_ HQUIC Connection,
        _In_opt_ void* Context,
        _Inout_ QUIC_CONNECTION_EVENT* Event
    );

    static QUIC_STATUS QUIC_API ClientStreamCallback(
        _In_ HQUIC Stream,
        _In_opt_ void* Context,
        _Inout_ QUIC_STREAM_EVENT* Event
    );

    // Internal helpers
    void Cleanup();
    bool InitializeMsQuic();
    bool LoadConfiguration();

    // State
    const QUIC_API_TABLE* msquic_api_;
    HQUIC registration_;
    HQUIC configuration_;
    HQUIC connection_;
    HQUIC stream_; // Primary stream for data transmission

    bool connected_;
    std::string remote_addr_;
    uint16_t remote_port_;
    
    std::mutex mutex_;
    std::function<void(const std::vector<uint8_t>&)> on_receive_;
    
    // Security/Session
    std::vector<uint8_t> session_key_;
};

} // namespace P2P