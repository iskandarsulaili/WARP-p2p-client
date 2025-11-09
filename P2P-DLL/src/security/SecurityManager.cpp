#include "../../include/SecurityManager.h"
#include "../../include/Logger.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstring>

namespace P2P {

struct SecurityManager::Impl {
    bool initialized = false;
    bool encryption_enabled = true;
    std::vector<uint8_t> encryption_key;

    // AES-256-GCM constants
    static constexpr size_t KEY_SIZE = 32;  // 256 bits
    static constexpr size_t IV_SIZE = 12;   // 96 bits (recommended for GCM)
    static constexpr size_t TAG_SIZE = 16;  // 128 bits
};

SecurityManager::SecurityManager() : impl_(std::make_unique<Impl>()) {
    LOG_DEBUG("SecurityManager created");
}

SecurityManager::~SecurityManager() {
    Shutdown();
}

bool SecurityManager::Initialize(bool encryption_enabled) {
    impl_->encryption_enabled = encryption_enabled;
    
    if (encryption_enabled) {
        // Generate random encryption key (32 bytes for AES-256)
        impl_->encryption_key.resize(32);
        if (RAND_bytes(reinterpret_cast<unsigned char*>(&impl_->encryption_key[0]), 32) != 1) {
            LOG_ERROR("Failed to generate encryption key");
            return false;
        }
    }
    
    impl_->initialized = true;
    LOG_INFO("SecurityManager initialized (encryption: " + std::string(encryption_enabled ? "ON" : "OFF") + ")");
    return true;
}

void SecurityManager::Shutdown() {
    impl_->encryption_key.clear();
    impl_->initialized = false;
}

bool SecurityManager::EncryptPacket(const uint8_t* data, size_t size, std::vector<uint8_t>& encrypted_out) {
    if (!impl_->encryption_enabled) {
        encrypted_out.assign(data, data + size);
        return true;
    }

    if (!impl_->initialized || impl_->encryption_key.empty()) {
        LOG_ERROR("SecurityManager not initialized or no encryption key");
        return false;
    }

    try {
        // Generate random IV (12 bytes for GCM)
        std::vector<uint8_t> iv(Impl::IV_SIZE);
        if (RAND_bytes(iv.data(), Impl::IV_SIZE) != 1) {
            LOG_ERROR("Failed to generate IV");
            return false;
        }

        // Create and initialize cipher context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            LOG_ERROR("Failed to create cipher context");
            return false;
        }

        // Initialize encryption operation with AES-256-GCM
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
            LOG_ERROR("Failed to initialize encryption");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        // Set IV length
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, Impl::IV_SIZE, nullptr) != 1) {
            LOG_ERROR("Failed to set IV length");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        // Initialize key and IV
        if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, impl_->encryption_key.data(), iv.data()) != 1) {
            LOG_ERROR("Failed to set key and IV");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        // Prepare output buffer: IV + ciphertext + tag
        encrypted_out.resize(Impl::IV_SIZE + size + Impl::TAG_SIZE);

        // Copy IV to output
        std::memcpy(encrypted_out.data(), iv.data(), Impl::IV_SIZE);

        // Encrypt data
        int len = 0;
        if (EVP_EncryptUpdate(ctx, encrypted_out.data() + Impl::IV_SIZE, &len, data, static_cast<int>(size)) != 1) {
            LOG_ERROR("Encryption failed");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        int ciphertext_len = len;

        // Finalize encryption
        if (EVP_EncryptFinal_ex(ctx, encrypted_out.data() + Impl::IV_SIZE + len, &len) != 1) {
            LOG_ERROR("Encryption finalization failed");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        ciphertext_len += len;

        // Get authentication tag
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, Impl::TAG_SIZE,
                                 encrypted_out.data() + Impl::IV_SIZE + ciphertext_len) != 1) {
            LOG_ERROR("Failed to get authentication tag");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        // Resize to actual size
        encrypted_out.resize(Impl::IV_SIZE + ciphertext_len + Impl::TAG_SIZE);

        EVP_CIPHER_CTX_free(ctx);
        LOG_DEBUG("Encrypted packet (" + std::to_string(size) + " -> " + std::to_string(encrypted_out.size()) + " bytes)");
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Encryption exception: " + std::string(e.what()));
        return false;
    }
}

bool SecurityManager::DecryptPacket(const uint8_t* data, size_t size, std::vector<uint8_t>& decrypted_out) {
    if (!impl_->encryption_enabled) {
        decrypted_out.assign(data, data + size);
        return true;
    }

    if (!impl_->initialized || impl_->encryption_key.empty()) {
        LOG_ERROR("SecurityManager not initialized or no encryption key");
        return false;
    }

    // Minimum size check: IV + TAG
    if (size < Impl::IV_SIZE + Impl::TAG_SIZE) {
        LOG_ERROR("Encrypted data too small");
        return false;
    }

    try {
        // Extract IV from the beginning
        const uint8_t* iv = data;

        // Calculate ciphertext length
        size_t ciphertext_len = size - Impl::IV_SIZE - Impl::TAG_SIZE;
        const uint8_t* ciphertext = data + Impl::IV_SIZE;

        // Extract tag from the end
        const uint8_t* tag = data + Impl::IV_SIZE + ciphertext_len;

        // Create and initialize cipher context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            LOG_ERROR("Failed to create cipher context");
            return false;
        }

        // Initialize decryption operation with AES-256-GCM
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
            LOG_ERROR("Failed to initialize decryption");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        // Set IV length
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, Impl::IV_SIZE, nullptr) != 1) {
            LOG_ERROR("Failed to set IV length");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        // Initialize key and IV
        if (EVP_DecryptInit_ex(ctx, nullptr, nullptr, impl_->encryption_key.data(), iv) != 1) {
            LOG_ERROR("Failed to set key and IV");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        // Prepare output buffer
        decrypted_out.resize(ciphertext_len);

        // Decrypt data
        int len = 0;
        if (EVP_DecryptUpdate(ctx, decrypted_out.data(), &len, ciphertext, static_cast<int>(ciphertext_len)) != 1) {
            LOG_ERROR("Decryption failed");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        int plaintext_len = len;

        // Set expected tag value
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, Impl::TAG_SIZE,
                                 const_cast<uint8_t*>(tag)) != 1) {
            LOG_ERROR("Failed to set authentication tag");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        // Finalize decryption (this verifies the tag)
        if (EVP_DecryptFinal_ex(ctx, decrypted_out.data() + len, &len) != 1) {
            LOG_ERROR("Decryption finalization failed - authentication tag mismatch");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        plaintext_len += len;

        // Resize to actual size
        decrypted_out.resize(plaintext_len);

        EVP_CIPHER_CTX_free(ctx);
        LOG_DEBUG("Decrypted packet (" + std::to_string(size) + " -> " + std::to_string(plaintext_len) + " bytes)");
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Decryption exception: " + std::string(e.what()));
        return false;
    }
}

bool SecurityManager::ValidatePacket(const uint8_t* data, size_t size) {
    if (!data || size == 0) {
        LOG_ERROR("Invalid packet: null data or zero size");
        return false;
    }

    // Minimum packet size check (at least 2 bytes for packet type)
    if (size < 2) {
        LOG_ERROR("Invalid packet: size too small (" + std::to_string(size) + " bytes)");
        return false;
    }

    // Maximum packet size check (prevent DoS attacks)
    constexpr size_t MAX_PACKET_SIZE = 1024 * 1024; // 1 MB
    if (size > MAX_PACKET_SIZE) {
        LOG_ERROR("Invalid packet: size too large (" + std::to_string(size) + " bytes)");
        return false;
    }

    // Read packet type (first 2 bytes, little-endian)
    uint16_t packet_type = static_cast<uint16_t>(data[0]) | (static_cast<uint16_t>(data[1]) << 8);

    // Validate packet type range (Ragnarok Online packet types are typically 0x0000-0x0FFF)
    if (packet_type > 0x0FFF) {
        LOG_WARN("Suspicious packet type: 0x" + std::to_string(packet_type));
        // Don't reject, just warn - some custom packets might use higher ranges
    }

    // If packet has length field (bytes 2-3), validate it
    if (size >= 4) {
        uint16_t declared_length = static_cast<uint16_t>(data[2]) | (static_cast<uint16_t>(data[3]) << 8);

        // Some packets have variable length with length field
        if (declared_length > 0 && declared_length != size) {
            LOG_ERROR("Packet length mismatch: declared=" + std::to_string(declared_length) +
                     ", actual=" + std::to_string(size));
            return false;
        }
    }

    LOG_DEBUG("Packet validated: type=0x" + std::to_string(packet_type) + ", size=" + std::to_string(size));
    return true;
}

bool SecurityManager::IsEncryptionEnabled() const {
    return impl_->encryption_enabled;
}

} // namespace P2P
