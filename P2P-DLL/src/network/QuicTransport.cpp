#include "../../include/QuicTransport.h"
#include "../../include/Logger.h"

namespace P2P {

#include <cstring>
#include <thread>
#include <mutex>
#include <vector>
#include <stdexcept>
#include <random>
#include <openssl/evp.h>
#include <openssl/rand.h>

// NOTE: In production, replace with a real QUIC library (e.g., msquic, quiche, etc.)
// Here, we simulate a secure, production-grade transport for demonstration.

QuicTransport::QuicTransport()
    : connected_(false), quic_session_(nullptr), remote_port_(0)
{
    LOG_DEBUG("QuicTransport created");
    session_key_.resize(32);
    if (!RAND_bytes(session_key_.data(), session_key_.size())) {
        throw std::runtime_error("Failed to generate session key");
    }
}

QuicTransport::~QuicTransport() {
    Disconnect();
    cleanup();
    LOG_DEBUG("QuicTransport destroyed");
}

bool QuicTransport::Connect(const std::string& address, uint16_t port) {
    std::lock_guard<std::mutex> lock(conn_mutex_);
    if (connected_) return true;
    remote_addr_ = address;
    remote_port_ = port;
    // Simulate QUIC session creation
    quic_session_ = reinterpret_cast<void*>(0xDEADBEEF); // placeholder
    connected_ = true;
    LOG_INFO("QuicTransport::Connect established to " + address + ":" + std::to_string(port));
    return true;
}

void QuicTransport::Disconnect() {
    std::lock_guard<std::mutex> lock(conn_mutex_);
    if (connected_) {
        // Simulate QUIC session cleanup
        cleanup();
        connected_ = false;
        LOG_INFO("QuicTransport::Disconnect completed");
    }
}

bool QuicTransport::SendData(const void* data, size_t size) {
    std::lock_guard<std::mutex> lock(conn_mutex_);
    if (!connected_ || !quic_session_) {
        LOG_ERROR("QuicTransport::SendData failed - not connected");
        return false;
    }
    // Encrypt and send data (simulate)
    std::vector<uint8_t> encrypted(size + 16); // 16 bytes for tag
    int outlen = 0;
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, session_key_.data(), nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    if (EVP_EncryptUpdate(ctx, encrypted.data(), &outlen, reinterpret_cast<const uint8_t*>(data), size) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    int tmplen = 0;
    if (EVP_EncryptFinal_ex(ctx, encrypted.data() + outlen, &tmplen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    outlen += tmplen;
    EVP_CIPHER_CTX_free(ctx);
    // Simulate network send: immediately call receive handler
    handle_receive(encrypted.data(), outlen);
    LOG_DEBUG("QuicTransport::SendData sent " + std::to_string(size) + " bytes (encrypted)");
    return true;
}

void QuicTransport::SetOnReceive(std::function<void(const std::vector<uint8_t>&)> callback) {
    std::lock_guard<std::mutex> lock(conn_mutex_);
    on_receive_ = std::move(callback);
    LOG_DEBUG("QuicTransport::SetOnReceive set");
}

bool QuicTransport::IsConnected() const {
    return connected_;
}

bool QuicTransport::validate_and_decrypt(const uint8_t* data, size_t size, std::vector<uint8_t>& out) {
    // Simulate packet validation and decryption
    if (size < 16) return false;
    out.resize(size - 16);
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, session_key_.data(), nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    int outlen = 0;
    if (EVP_DecryptUpdate(ctx, out.data(), &outlen, data, size - 16) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    int tmplen = 0;
    if (EVP_DecryptFinal_ex(ctx, out.data() + outlen, &tmplen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    out.resize(outlen + tmplen);
    EVP_CIPHER_CTX_free(ctx);
    return true;
}

void QuicTransport::handle_receive(const uint8_t* data, size_t size) {
    std::vector<uint8_t> decrypted;
    if (validate_and_decrypt(data, size, decrypted)) {
        if (on_receive_) {
            on_receive_(decrypted);
        }
    } else {
        LOG_ERROR("QuicTransport::handle_receive: packet validation/decryption failed");
    }
}

void QuicTransport::cleanup() {
    quic_session_ = nullptr;
    // Clean up any other resources if needed
}

} // namespace P2P