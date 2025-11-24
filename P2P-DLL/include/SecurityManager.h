#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include <string>

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

    // ED25519: Load private key from file
    bool LoadED25519Key(const std::string& key_path);

    // ED25519: Sign outbound packet
    bool SignPacketED25519(const uint8_t* data, size_t size, std::vector<uint8_t>& signature_out);

    // ED25519: Check if signature is enabled
    bool IsSignatureEnabled() const;

    // ED25519: Verify packet signature
    bool VerifyPacketED25519(const uint8_t* data, size_t size, const uint8_t* signature);

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
     * @param compression_manager The CompressionManager shared_ptr (does not take ownership)
     * @note This stores a weak reference to avoid circular dependencies
     */
    void SetCompressionManager(std::shared_ptr<CompressionManager> compression_manager);

    // ECDHE Key Exchange Methods
    /**
     * Generate ECDHE keypair for key exchange
     * Uses secp256r1 (NIST P-256) curve
     * @return true if keypair generation succeeded
     */
    bool GenerateECDHKeypair();

    /**
     * Get the public key for transmission to peer
     * @return Public key in DER format, empty if not generated
     */
    std::vector<uint8_t> GetPublicKey() const;

    /**
     * Derive shared encryption key from peer's public key
     * Uses ECDH to compute shared secret, then HKDF-SHA256 to derive AES-256 key
     * @param peer_public_key Peer's public key in DER format
     * @return true if key derivation succeeded
     */
    bool DeriveSharedKey(const std::vector<uint8_t>& peer_public_key);

    /**
     * Check if encryption key is ready (either generated or derived)
     * @return true if encryption key is available
     */
    bool IsKeyReady() const;

private:
    // Pimpl idiom for implementation details
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace P2P
