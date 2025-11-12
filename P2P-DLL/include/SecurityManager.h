#pragma once

#include <memory>
#include <vector>
#include <cstdint>

// Forward declaration
namespace P2P {
class CompressionManager;
}

namespace P2P {

/**
 * SecurityManager - Handles encryption and decryption of packets
 */
class SecurityManager {
public:
    SecurityManager();
    ~SecurityManager();

    // Disable copy and move
    SecurityManager(const SecurityManager&) = delete;
    SecurityManager& operator=(const SecurityManager&) = delete;
    SecurityManager(SecurityManager&&) = delete;
    SecurityManager& operator=(SecurityManager&&) = delete;

    /**
     * Initialize the security manager
     * @param encryption_enabled Whether encryption is enabled
     * @return true if initialization succeeded
     */
    bool Initialize(bool encryption_enabled);

    /**
     * Shutdown the security manager
     */
    void Shutdown();

    /**
     * Encrypt a packet
     * @param data Input data
     * @param size Data size
     * @param encrypted_out Output encrypted data
     * @return true if encryption succeeded
     */
    bool EncryptPacket(const uint8_t* data, size_t size, std::vector<uint8_t>& encrypted_out);

    /**
     * Decrypt a packet
     * @param data Input encrypted data
     * @param size Data size
     * @param decrypted_out Output decrypted data
     * @return true if decryption succeeded
     */
    bool DecryptPacket(const uint8_t* data, size_t size, std::vector<uint8_t>& decrypted_out);

    /**
     * Validate a packet
     * @param data Input data
     * @param size Data size
     * @return true if packet is valid
     */
    bool ValidatePacket(const uint8_t* data, size_t size);

    /**
     * Check if encryption is enabled
     * @return true if encryption is enabled
     */
    bool IsEncryptionEnabled() const;

    /**
     * Set CompressionManager for packet compression
     * @param compression_manager The CompressionManager instance
     */
    void SetCompressionManager(CompressionManager* compression_manager);

private:
    // Pimpl idiom for implementation details
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace P2P
