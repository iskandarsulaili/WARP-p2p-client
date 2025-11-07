#ifndef P2P_ENCRYPTION_HPP
#define P2P_ENCRYPTION_HPP

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <cstdint>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

/**
 * @brief P2P Encryption Manager
 * 
 * Provides end-to-end encryption for P2P data channels using AES-256-GCM.
 * Implements key exchange, encryption, decryption, and key rotation.
 * 
 * Security Features:
 * - AES-256-GCM authenticated encryption
 * - Perfect forward secrecy with key rotation
 * - HMAC-SHA256 for message authentication
 * - Secure random key generation
 * 
 * Thread Safety: All public methods are thread-safe
 * 
 * Usage:
 *   P2PEncryption encryption;
 *   encryption.initialize();
 *   auto encrypted = encryption.encrypt(data);
 *   auto decrypted = encryption.decrypt(encrypted);
 */
class P2PEncryption {
public:
    /**
     * @brief Encryption algorithm enumeration
     */
    enum class Algorithm {
        AES_256_GCM,    // AES-256 in GCM mode (recommended)
        AES_256_CBC,    // AES-256 in CBC mode
        CHACHA20_POLY1305  // ChaCha20-Poly1305 (alternative)
    };
    
    /**
     * @brief Encrypted data structure
     */
    struct EncryptedData {
        std::vector<uint8_t> ciphertext;    // Encrypted data
        std::vector<uint8_t> iv;            // Initialization vector
        std::vector<uint8_t> tag;           // Authentication tag (GCM mode)
        uint64_t timestamp;                 // Encryption timestamp
        uint32_t key_version;               // Key version for rotation
    };
    
    /**
     * @brief Key rotation configuration
     */
    struct KeyRotationConfig {
        uint32_t rotation_interval_seconds; // How often to rotate keys
        uint32_t max_messages_per_key;      // Max messages before rotation
        bool enable_auto_rotation;          // Enable automatic rotation
    };
    
    /**
     * @brief Constructor
     */
    P2PEncryption();
    
    /**
     * @brief Destructor - securely wipes keys
     */
    ~P2PEncryption();
    
    // Disable copy
    P2PEncryption(const P2PEncryption&) = delete;
    P2PEncryption& operator=(const P2PEncryption&) = delete;
    
    /**
     * @brief Initialize encryption system
     * @param algorithm Encryption algorithm to use
     * @param rotation_config Key rotation configuration
     * @return true if initialization successful
     */
    bool initialize(Algorithm algorithm = Algorithm::AES_256_GCM,
                   const KeyRotationConfig& rotation_config = {});
    
    /**
     * @brief Generate new encryption key
     * @return true if key generated successfully
     */
    bool generate_key();
    
    /**
     * @brief Set encryption key (for key exchange)
     * @param key Encryption key (32 bytes for AES-256)
     * @return true if key set successfully
     */
    bool set_key(const std::vector<uint8_t>& key);
    
    /**
     * @brief Get current encryption key (for key exchange)
     * @return Current encryption key
     */
    std::vector<uint8_t> get_key() const;
    
    /**
     * @brief Encrypt data
     * @param plaintext Data to encrypt
     * @return Encrypted data structure
     */
    EncryptedData encrypt(const std::vector<uint8_t>& plaintext);
    
    /**
     * @brief Decrypt data
     * @param encrypted Encrypted data structure
     * @return Decrypted plaintext or empty vector on failure
     */
    std::vector<uint8_t> decrypt(const EncryptedData& encrypted);
    
    /**
     * @brief Encrypt string message
     * @param message String to encrypt
     * @return Encrypted data structure
     */
    EncryptedData encrypt_message(const std::string& message);
    
    /**
     * @brief Decrypt to string message
     * @param encrypted Encrypted data structure
     * @return Decrypted string or empty string on failure
     */
    std::string decrypt_message(const EncryptedData& encrypted);
    
    /**
     * @brief Rotate encryption key
     * @return true if rotation successful
     */
    bool rotate_key();
    
    /**
     * @brief Check if key rotation is needed
     * @return true if rotation recommended
     */
    bool should_rotate_key() const;
    
    /**
     * @brief Serialize encrypted data for transmission
     * @param encrypted Encrypted data structure
     * @return Serialized binary data
     */
    static std::vector<uint8_t> serialize(const EncryptedData& encrypted);
    
    /**
     * @brief Deserialize encrypted data from transmission
     * @param data Serialized binary data
     * @return Encrypted data structure
     */
    static EncryptedData deserialize(const std::vector<uint8_t>& data);
    
    /**
     * @brief Compute HMAC-SHA256 for message authentication
     * @param data Data to authenticate
     * @param key HMAC key
     * @return HMAC digest
     */
    static std::vector<uint8_t> compute_hmac(const std::vector<uint8_t>& data,
                                             const std::vector<uint8_t>& key);
    
    /**
     * @brief Verify HMAC-SHA256
     * @param data Data to verify
     * @param hmac HMAC to verify against
     * @param key HMAC key
     * @return true if HMAC is valid
     */
    static bool verify_hmac(const std::vector<uint8_t>& data,
                           const std::vector<uint8_t>& hmac,
                           const std::vector<uint8_t>& key);
    
    /**
     * @brief Get encryption statistics
     */
    struct Statistics {
        uint64_t messages_encrypted;
        uint64_t messages_decrypted;
        uint64_t encryption_failures;
        uint64_t decryption_failures;
        uint32_t current_key_version;
        uint64_t key_age_seconds;
    };
    
    Statistics get_statistics() const;

private:
    Algorithm algorithm_;
    KeyRotationConfig rotation_config_;
    
    // Encryption keys
    std::vector<uint8_t> current_key_;
    std::vector<uint8_t> previous_key_;  // For decrypting old messages
    uint32_t key_version_;
    uint64_t key_creation_time_;
    
    // Statistics
    mutable Statistics stats_;
    
    // Thread safety
    mutable std::mutex mutex_;
    
    // OpenSSL context
    EVP_CIPHER_CTX* encrypt_ctx_;
    EVP_CIPHER_CTX* decrypt_ctx_;
    
    // Internal methods
    bool encrypt_aes_gcm(const std::vector<uint8_t>& plaintext,
                        const std::vector<uint8_t>& key,
                        const std::vector<uint8_t>& iv,
                        std::vector<uint8_t>& ciphertext,
                        std::vector<uint8_t>& tag);
    
    bool decrypt_aes_gcm(const std::vector<uint8_t>& ciphertext,
                        const std::vector<uint8_t>& key,
                        const std::vector<uint8_t>& iv,
                        const std::vector<uint8_t>& tag,
                        std::vector<uint8_t>& plaintext);
    
    std::vector<uint8_t> generate_iv() const;
    void secure_wipe(std::vector<uint8_t>& data);
    uint64_t get_current_timestamp() const;
    
    void log_info(const std::string& message);
    void log_error(const std::string& message);
};

#endif // P2P_ENCRYPTION_HPP

