#include "P2PEncryption.hpp"
#include <iostream>
#include <cstring>
#include <chrono>
#include <openssl/err.h>

P2PEncryption::P2PEncryption()
    : algorithm_(Algorithm::AES_256_GCM)
    , key_version_(0)
    , key_creation_time_(0)
    , encrypt_ctx_(nullptr)
    , decrypt_ctx_(nullptr)
{
    log_info("P2P Encryption created");
    
    // Initialize statistics
    stats_.messages_encrypted = 0;
    stats_.messages_decrypted = 0;
    stats_.encryption_failures = 0;
    stats_.decryption_failures = 0;
    stats_.current_key_version = 0;
    stats_.key_age_seconds = 0;
    
    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    
    // Create encryption contexts
    encrypt_ctx_ = EVP_CIPHER_CTX_new();
    decrypt_ctx_ = EVP_CIPHER_CTX_new();
}

P2PEncryption::~P2PEncryption() {
    // Securely wipe keys
    secure_wipe(current_key_);
    secure_wipe(previous_key_);
    
    // Clean up OpenSSL contexts
    if (encrypt_ctx_) {
        EVP_CIPHER_CTX_free(encrypt_ctx_);
    }
    if (decrypt_ctx_) {
        EVP_CIPHER_CTX_free(decrypt_ctx_);
    }
    
    log_info("P2P Encryption destroyed");
}

bool P2PEncryption::initialize(Algorithm algorithm, const KeyRotationConfig& rotation_config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    algorithm_ = algorithm;
    rotation_config_ = rotation_config;
    
    // Set default rotation config if not provided
    if (rotation_config_.rotation_interval_seconds == 0) {
        rotation_config_.rotation_interval_seconds = 3600;  // 1 hour
    }
    if (rotation_config_.max_messages_per_key == 0) {
        rotation_config_.max_messages_per_key = 100000;
    }
    
    log_info("Initializing P2P Encryption");
    log_info("Algorithm: AES-256-GCM");
    log_info("Key rotation interval: " + std::to_string(rotation_config_.rotation_interval_seconds) + "s");
    
    // Generate initial key
    if (!generate_key()) {
        log_error("Failed to generate initial key");
        return false;
    }
    
    log_info("P2P Encryption initialized successfully");
    return true;
}

bool P2PEncryption::generate_key() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    log_info("Generating new encryption key");
    
    // Save previous key for decrypting old messages
    if (!current_key_.empty()) {
        previous_key_ = current_key_;
    }
    
    // Generate 256-bit (32-byte) key
    current_key_.resize(32);
    
    if (RAND_bytes(current_key_.data(), 32) != 1) {
        log_error("Failed to generate random key");
        stats_.encryption_failures++;
        return false;
    }
    
    key_version_++;
    key_creation_time_ = get_current_timestamp();
    stats_.current_key_version = key_version_;
    
    log_info("New key generated (version: " + std::to_string(key_version_) + ")");
    return true;
}

bool P2PEncryption::set_key(const std::vector<uint8_t>& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (key.size() != 32) {
        log_error("Invalid key size: " + std::to_string(key.size()) + " (expected 32)");
        return false;
    }
    
    // Save previous key
    if (!current_key_.empty()) {
        previous_key_ = current_key_;
    }
    
    current_key_ = key;
    key_version_++;
    key_creation_time_ = get_current_timestamp();
    stats_.current_key_version = key_version_;
    
    log_info("Encryption key set (version: " + std::to_string(key_version_) + ")");
    return true;
}

std::vector<uint8_t> P2PEncryption::get_key() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_key_;
}

P2PEncryption::EncryptedData P2PEncryption::encrypt(const std::vector<uint8_t>& plaintext) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    EncryptedData result;
    result.timestamp = get_current_timestamp();
    result.key_version = key_version_;
    
    if (current_key_.empty()) {
        log_error("No encryption key set");
        stats_.encryption_failures++;
        return result;
    }
    
    // Generate random IV (12 bytes for GCM)
    result.iv = generate_iv();
    
    // Encrypt using AES-256-GCM
    if (!encrypt_aes_gcm(plaintext, current_key_, result.iv, result.ciphertext, result.tag)) {
        log_error("Encryption failed");
        stats_.encryption_failures++;
        return result;
    }
    
    stats_.messages_encrypted++;
    
    // Check if key rotation needed
    if (rotation_config_.enable_auto_rotation && should_rotate_key()) {
        log_info("Auto-rotating key");
        generate_key();
    }
    
    return result;
}

std::vector<uint8_t> P2PEncryption::decrypt(const EncryptedData& encrypted) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<uint8_t> plaintext;
    
    if (current_key_.empty()) {
        log_error("No decryption key set");
        stats_.decryption_failures++;
        return plaintext;
    }
    
    // Try current key first
    if (decrypt_aes_gcm(encrypted.ciphertext, current_key_, encrypted.iv, encrypted.tag, plaintext)) {
        stats_.messages_decrypted++;
        return plaintext;
    }
    
    // Try previous key if available (for key rotation)
    if (!previous_key_.empty()) {
        if (decrypt_aes_gcm(encrypted.ciphertext, previous_key_, encrypted.iv, encrypted.tag, plaintext)) {
            log_info("Decrypted with previous key");
            stats_.messages_decrypted++;
            return plaintext;
        }
    }
    
    log_error("Decryption failed");
    stats_.decryption_failures++;
    return plaintext;
}

P2PEncryption::EncryptedData P2PEncryption::encrypt_message(const std::string& message) {
    std::vector<uint8_t> plaintext(message.begin(), message.end());
    return encrypt(plaintext);
}

std::string P2PEncryption::decrypt_message(const EncryptedData& encrypted) {
    auto plaintext = decrypt(encrypted);
    if (plaintext.empty()) {
        return "";
    }
    return std::string(plaintext.begin(), plaintext.end());
}

bool P2PEncryption::rotate_key() {
    log_info("Manually rotating encryption key");
    return generate_key();
}

bool P2PEncryption::should_rotate_key() const {
    // Check time-based rotation
    uint64_t key_age = get_current_timestamp() - key_creation_time_;
    if (key_age >= rotation_config_.rotation_interval_seconds) {
        return true;
    }

    // Check message count-based rotation
    if (stats_.messages_encrypted >= rotation_config_.max_messages_per_key) {
        return true;
    }

    return false;
}

std::vector<uint8_t> P2PEncryption::serialize(const EncryptedData& encrypted) {
    std::vector<uint8_t> result;

    // Format: [iv_size(2)][iv][tag_size(2)][tag][ciphertext_size(4)][ciphertext][timestamp(8)][key_version(4)]

    // IV size and data
    uint16_t iv_size = static_cast<uint16_t>(encrypted.iv.size());
    result.push_back(iv_size & 0xFF);
    result.push_back((iv_size >> 8) & 0xFF);
    result.insert(result.end(), encrypted.iv.begin(), encrypted.iv.end());

    // Tag size and data
    uint16_t tag_size = static_cast<uint16_t>(encrypted.tag.size());
    result.push_back(tag_size & 0xFF);
    result.push_back((tag_size >> 8) & 0xFF);
    result.insert(result.end(), encrypted.tag.begin(), encrypted.tag.end());

    // Ciphertext size and data
    uint32_t ct_size = static_cast<uint32_t>(encrypted.ciphertext.size());
    result.push_back(ct_size & 0xFF);
    result.push_back((ct_size >> 8) & 0xFF);
    result.push_back((ct_size >> 16) & 0xFF);
    result.push_back((ct_size >> 24) & 0xFF);
    result.insert(result.end(), encrypted.ciphertext.begin(), encrypted.ciphertext.end());

    // Timestamp
    for (int i = 0; i < 8; i++) {
        result.push_back((encrypted.timestamp >> (i * 8)) & 0xFF);
    }

    // Key version
    for (int i = 0; i < 4; i++) {
        result.push_back((encrypted.key_version >> (i * 8)) & 0xFF);
    }

    return result;
}

P2PEncryption::EncryptedData P2PEncryption::deserialize(const std::vector<uint8_t>& data) {
    EncryptedData result;

    if (data.size() < 20) {  // Minimum size
        return result;
    }

    size_t offset = 0;

    // Read IV
    uint16_t iv_size = data[offset] | (data[offset + 1] << 8);
    offset += 2;
    result.iv.assign(data.begin() + offset, data.begin() + offset + iv_size);
    offset += iv_size;

    // Read tag
    uint16_t tag_size = data[offset] | (data[offset + 1] << 8);
    offset += 2;
    result.tag.assign(data.begin() + offset, data.begin() + offset + tag_size);
    offset += tag_size;

    // Read ciphertext
    uint32_t ct_size = data[offset] | (data[offset + 1] << 8) |
                      (data[offset + 2] << 16) | (data[offset + 3] << 24);
    offset += 4;
    result.ciphertext.assign(data.begin() + offset, data.begin() + offset + ct_size);
    offset += ct_size;

    // Read timestamp
    result.timestamp = 0;
    for (int i = 0; i < 8; i++) {
        result.timestamp |= static_cast<uint64_t>(data[offset + i]) << (i * 8);
    }
    offset += 8;

    // Read key version
    result.key_version = 0;
    for (int i = 0; i < 4; i++) {
        result.key_version |= static_cast<uint32_t>(data[offset + i]) << (i * 8);
    }

    return result;
}

std::vector<uint8_t> P2PEncryption::compute_hmac(const std::vector<uint8_t>& data,
                                                 const std::vector<uint8_t>& key) {
    std::vector<uint8_t> hmac(SHA256_DIGEST_LENGTH);

    unsigned int hmac_len = 0;
    HMAC(EVP_sha256(), key.data(), key.size(),
         data.data(), data.size(),
         hmac.data(), &hmac_len);

    return hmac;
}

bool P2PEncryption::verify_hmac(const std::vector<uint8_t>& data,
                                const std::vector<uint8_t>& hmac,
                                const std::vector<uint8_t>& key) {
    auto computed_hmac = compute_hmac(data, key);

    if (computed_hmac.size() != hmac.size()) {
        return false;
    }

    // Constant-time comparison to prevent timing attacks
    int result = 0;
    for (size_t i = 0; i < hmac.size(); i++) {
        result |= computed_hmac[i] ^ hmac[i];
    }

    return result == 0;
}

P2PEncryption::Statistics P2PEncryption::get_statistics() const {
    std::lock_guard<std::mutex> lock(mutex_);

    Statistics stats = stats_;
    stats.key_age_seconds = get_current_timestamp() - key_creation_time_;

    return stats;
}

bool P2PEncryption::encrypt_aes_gcm(const std::vector<uint8_t>& plaintext,
                                   const std::vector<uint8_t>& key,
                                   const std::vector<uint8_t>& iv,
                                   std::vector<uint8_t>& ciphertext,
                                   std::vector<uint8_t>& tag) {
    int len = 0;
    int ciphertext_len = 0;

    // Initialize encryption
    if (EVP_EncryptInit_ex(encrypt_ctx_, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
        return false;
    }

    // Set IV length
    if (EVP_CIPHER_CTX_ctrl(encrypt_ctx_, EVP_CTRL_GCM_SET_IVLEN, iv.size(), nullptr) != 1) {
        return false;
    }

    // Initialize key and IV
    if (EVP_EncryptInit_ex(encrypt_ctx_, nullptr, nullptr, key.data(), iv.data()) != 1) {
        return false;
    }

    // Encrypt plaintext
    ciphertext.resize(plaintext.size() + EVP_CIPHER_block_size(EVP_aes_256_gcm()));

    if (EVP_EncryptUpdate(encrypt_ctx_, ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1) {
        return false;
    }
    ciphertext_len = len;

    // Finalize encryption
    if (EVP_EncryptFinal_ex(encrypt_ctx_, ciphertext.data() + len, &len) != 1) {
        return false;
    }
    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);

    // Get authentication tag
    tag.resize(16);
    if (EVP_CIPHER_CTX_ctrl(encrypt_ctx_, EVP_CTRL_GCM_GET_TAG, 16, tag.data()) != 1) {
        return false;
    }

    return true;
}

bool P2PEncryption::decrypt_aes_gcm(const std::vector<uint8_t>& ciphertext,
                                   const std::vector<uint8_t>& key,
                                   const std::vector<uint8_t>& iv,
                                   const std::vector<uint8_t>& tag,
                                   std::vector<uint8_t>& plaintext) {
    int len = 0;
    int plaintext_len = 0;

    // Initialize decryption
    if (EVP_DecryptInit_ex(decrypt_ctx_, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
        return false;
    }

    // Set IV length
    if (EVP_CIPHER_CTX_ctrl(decrypt_ctx_, EVP_CTRL_GCM_SET_IVLEN, iv.size(), nullptr) != 1) {
        return false;
    }

    // Initialize key and IV
    if (EVP_DecryptInit_ex(decrypt_ctx_, nullptr, nullptr, key.data(), iv.data()) != 1) {
        return false;
    }

    // Decrypt ciphertext
    plaintext.resize(ciphertext.size());

    if (EVP_DecryptUpdate(decrypt_ctx_, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) {
        return false;
    }
    plaintext_len = len;

    // Set expected tag
    if (EVP_CIPHER_CTX_ctrl(decrypt_ctx_, EVP_CTRL_GCM_SET_TAG, tag.size(), const_cast<uint8_t*>(tag.data())) != 1) {
        return false;
    }

    // Finalize decryption (verifies tag)
    if (EVP_DecryptFinal_ex(decrypt_ctx_, plaintext.data() + len, &len) != 1) {
        return false;
    }
    plaintext_len += len;
    plaintext.resize(plaintext_len);

    return true;
}

std::vector<uint8_t> P2PEncryption::generate_iv() const {
    std::vector<uint8_t> iv(12);  // 96 bits for GCM
    RAND_bytes(iv.data(), 12);
    return iv;
}

void P2PEncryption::secure_wipe(std::vector<uint8_t>& data) {
    if (!data.empty()) {
        OPENSSL_cleanse(data.data(), data.size());
        data.clear();
    }
}

uint64_t P2PEncryption::get_current_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

void P2PEncryption::log_info(const std::string& message) {
    std::cout << "[P2PEncryption] INFO: " << message << std::endl;
}

void P2PEncryption::log_error(const std::string& message) {
    std::cerr << "[P2PEncryption] ERROR: " << message << std::endl;
}

