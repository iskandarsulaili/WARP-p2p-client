#include "../../include/QuicTransport.h"
#include "../../include/Logger.h"
#include <stdexcept>
#include <vector>
#include <iostream>

namespace P2P {

// Helper to convert std::string to C-string safely
inline const char* SafeStr(const std::string& s) { return s.c_str(); }

QuicTransport::QuicTransport()
    : msquic_api_(nullptr)
    , registration_(nullptr)
    , configuration_(nullptr)
    , connection_(nullptr)
    , stream_(nullptr)
    , connected_(false)
    , remote_port_(0)
{
    if (!InitializeMsQuic()) {
        LOG_ERROR("Failed to initialize MsQuic");
        throw std::runtime_error("MsQuic initialization failed");
    }
    LOG_INFO("QuicTransport initialized with MsQuic");
}

QuicTransport::~QuicTransport() {
    Disconnect();
    Cleanup();
    if (msquic_api_) {
        if (registration_) {
            msquic_api_->RegistrationClose(registration_);
        }
        // msquic_api_->Close(msquic_api_); // No Close method in API table
    }
}

bool QuicTransport::InitializeMsQuic() {
    QUIC_STATUS status = MsQuicOpen2(&msquic_api_);
    if (QUIC_FAILED(status)) {
        LOG_ERROR("MsQuicOpen2 failed: " + std::to_string(status));
        return false;
    }

    // Create Registration
    QUIC_REGISTRATION_CONFIG regConfig = { "warp-p2p-client", QUIC_EXECUTION_PROFILE_LOW_LATENCY };
    status = msquic_api_->RegistrationOpen(&regConfig, &registration_);
    if (QUIC_FAILED(status)) {
        LOG_ERROR("RegistrationOpen failed: " + std::to_string(status));
        return false;
    }

    return LoadConfiguration();
}

bool QuicTransport::LoadConfiguration() {
    QUIC_SETTINGS settings = {0};
    settings.IdleTimeoutMs = 30000;
    settings.IsSet.IdleTimeoutMs = TRUE;
    settings.ServerResumptionLevel = QUIC_SERVER_RESUME_AND_ZERORTT;
    settings.IsSet.ServerResumptionLevel = TRUE;
    settings.PeerBidiStreamCount = 1;
    settings.IsSet.PeerBidiStreamCount = TRUE;

    QUIC_CREDENTIAL_CONFIG credConfig = {0};
    credConfig.Type = QUIC_CREDENTIAL_TYPE_NONE;
    credConfig.Flags = QUIC_CREDENTIAL_FLAG_CLIENT;
    credConfig.Flags |= QUIC_CREDENTIAL_FLAG_NO_CERTIFICATE_VALIDATION;

    // Define ALPN
    QUIC_BUFFER alpn;
    alpn.Buffer = (uint8_t*)"warp-p2p";
    alpn.Length = 8;

    QUIC_STATUS status = msquic_api_->ConfigurationOpen(
        registration_,
        &alpn,
        1,
        &settings,
        sizeof(settings),
        nullptr,
        &configuration_
    );

    if (QUIC_FAILED(status)) {
        LOG_ERROR("ConfigurationOpen failed: " + std::to_string(status));
        return false;
    }

    status = msquic_api_->ConfigurationLoadCredential(
        configuration_,
        &credConfig
    );

    if (QUIC_FAILED(status)) {
        LOG_ERROR("ConfigurationLoadCredential failed: " + std::to_string(status));
        return false;
    }

    return true;
}

bool QuicTransport::Connect(const std::string& address, uint16_t port) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (connected_) return true;

    remote_addr_ = address;
    remote_port_ = port;

    QUIC_STATUS status = msquic_api_->ConnectionOpen(
        registration_,
        ClientConnectionCallback,
        this,
        &connection_
    );

    if (QUIC_FAILED(status)) {
        LOG_ERROR("ConnectionOpen failed: " + std::to_string(status));
        return false;
    }

    // Start the connection
    status = msquic_api_->ConnectionStart(
        connection_,
        configuration_,
        QUIC_ADDRESS_FAMILY_UNSPEC,
        address.c_str(),
        port
    );

    if (QUIC_FAILED(status)) {
        LOG_ERROR("ConnectionStart failed: " + std::to_string(status));
        msquic_api_->ConnectionClose(connection_);
        connection_ = nullptr;
        return false;
    }

    LOG_INFO("Connecting to " + address + ":" + std::to_string(port) + "...");
    return true;
}

void QuicTransport::Disconnect() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (connection_) {
        msquic_api_->ConnectionShutdown(connection_, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
        // ConnectionClose will be called in cleanup or destructor, 
        // but Shutdown initiates the teardown.
        // We can't close immediately if we want to send the shutdown frame.
        // For simplicity in this DLL, we'll just shutdown.
    }
    connected_ = false;
}

void QuicTransport::Cleanup() {
    if (stream_) {
        msquic_api_->StreamClose(stream_);
        stream_ = nullptr;
    }
    if (connection_) {
        msquic_api_->ConnectionClose(connection_);
        connection_ = nullptr;
    }
}

bool QuicTransport::SendData(const void* data, size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!connected_ || !stream_) {
        LOG_ERROR("Cannot send data: Not connected or stream not open");
        return false;
    }

    auto* buf = new QUIC_BUFFER();
    buf->Length = static_cast<uint32_t>(size);
    buf->Buffer = new uint8_t[size];
    memcpy(buf->Buffer, data, size);

    // Send with QUIC_SEND_FLAG_NONE. 
    // Note: We need to manage the buffer lifetime. 
    // Usually we pass the buffer context to the callback to delete it.
    // For this implementation, we'll use a simple wrapper or just leak for a microsecond 
    // (actually we MUST handle it in the callback).
    
    // To handle buffer cleanup, we'll attach it to the context or use a specific pattern.
    // For simplicity here, we'll assume the callback handles 'SendComplete'.
    
    QUIC_STATUS status = msquic_api_->StreamSend(
        stream_,
        buf,
        1,
        QUIC_SEND_FLAG_NONE,
        buf // Context
    );

    if (QUIC_FAILED(status)) {
        LOG_ERROR("StreamSend failed: " + std::to_string(status));
        delete[] buf->Buffer;
        delete buf;
        return false;
    }

    return true;
}

void QuicTransport::SetOnReceive(std::function<void(const std::vector<uint8_t>&)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    on_receive_ = callback;
}

bool QuicTransport::IsConnected() const {
    return connected_;
}

// Callbacks

_IRQL_requires_max_(DISPATCH_LEVEL)
_Function_class_(QUIC_CONNECTION_CALLBACK)
QUIC_STATUS QUIC_API QuicTransport::ClientConnectionCallback(
    _In_ HQUIC Connection,
    _In_opt_ void* Context,
    _Inout_ QUIC_CONNECTION_EVENT* Event
) {
    auto* pThis = static_cast<QuicTransport*>(Context);
    
    switch (Event->Type) {
    case QUIC_CONNECTION_EVENT_CONNECTED:
        LOG_INFO("QUIC Connected!");
        pThis->connected_ = true;
        
        // Open a stream for data
        {
            QUIC_STATUS status = pThis->msquic_api_->StreamOpen(
                Connection,
                QUIC_STREAM_OPEN_FLAG_NONE,
                ClientStreamCallback,
                pThis,
                &pThis->stream_
            );
            if (QUIC_SUCCEEDED(status)) {
                LOG_INFO("Stream created");
                pThis->msquic_api_->StreamStart(pThis->stream_, QUIC_STREAM_START_FLAG_IMMEDIATE);
            } else {
                LOG_ERROR("StreamOpen failed");
            }
        }
        break;
        
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
        LOG_ERROR("Connection shutdown by transport: " + std::to_string(Event->SHUTDOWN_INITIATED_BY_TRANSPORT.Status));
        pThis->connected_ = false;
        break;
        
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
        LOG_INFO("Connection shutdown by peer");
        pThis->connected_ = false;
        break;
        
    case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
        LOG_INFO("Connection shutdown complete");
        pThis->connected_ = false;
        break;
        
    default:
        break;
    }
    return QUIC_STATUS_SUCCESS;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
_Function_class_(QUIC_STREAM_CALLBACK)
QUIC_STATUS QUIC_API QuicTransport::ClientStreamCallback(
    _In_ HQUIC Stream,
    _In_opt_ void* Context,
    _Inout_ QUIC_STREAM_EVENT* Event
) {
    (void)Stream;
    auto* pThis = static_cast<QuicTransport*>(Context);

    switch (Event->Type) {
    case QUIC_STREAM_EVENT_RECEIVE:
        // Handle received data
        {
            size_t totalLength = 0;
            for (uint32_t i = 0; i < Event->RECEIVE.BufferCount; ++i) {
                totalLength += Event->RECEIVE.Buffers[i].Length;
            }
            
            std::vector<uint8_t> data;
            data.reserve(totalLength);
            
            for (uint32_t i = 0; i < Event->RECEIVE.BufferCount; ++i) {
                const auto& buf = Event->RECEIVE.Buffers[i];
                data.insert(data.end(), buf.Buffer, buf.Buffer + buf.Length);
            }
            
            if (pThis->on_receive_) {
                pThis->on_receive_(data);
            }
        }
        return QUIC_STATUS_SUCCESS;

    case QUIC_STREAM_EVENT_SEND_COMPLETE:
        // Clean up buffer we allocated in SendData
        {
            auto* buf = static_cast<QUIC_BUFFER*>(Event->SEND_COMPLETE.ClientContext);
            if (buf) {
                delete[] buf->Buffer;
                delete buf;
            }
        }
        break;
        
    case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN:
        LOG_INFO("Stream peer send shutdown");
        break;
        
    case QUIC_STREAM_EVENT_PEER_SEND_ABORTED:
        LOG_INFO("Stream peer send aborted");
        break;
        
    case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
        LOG_INFO("Stream shutdown complete");
        break;
        
    default:
        break;
    }
    return QUIC_STATUS_SUCCESS;
}

} // namespace P2P
