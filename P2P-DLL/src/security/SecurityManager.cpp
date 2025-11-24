#include "../../include/SecurityManager.h"
#include "../../include/CompressionManager.h"
#include "../../include/Logger.h"
#include "../../include/Types.h"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/kdf.h>
#include <sodium.h>
#include <cstring>
#include <vector>
#include <memory>
#include <mutex>
#include <fstream>

namespace P2P {

struct SecurityManager::Impl {
    bool initialized = false;
    bool encryption_enabled = false;
    std::vector<uint8_t> encryption_key;
    std::shared_ptr<CompressionManager> compression_manager;

    // ED25519
    bool signature_enabled = true;
    std::vector<uint8_t> ed25519_private_key; // 64 bytes (seed + key)
    std::vector<uint8_t> ed25519_public_key;  // 32 bytes
    std::mutex ed25519_mutex;

    // ECDHE Key Exchange
    EVP_PKEY* ecdh_keypair = nullptr;
    bool key_derived = false;
    std::mutex ecdhe_mutex;

    static constexpr size_t IV_SIZE = 12;
    static constexpr size_t TAG_SIZE = 16;
    static constexpr size_t ED25519_SIG_SIZE = 64;
    static constexpr size_t ED25519_PUBKEY_SIZE = 32;
    static constexpr size_t ED25519_PRIVKEY_SIZE = 64;
    static constexpr size_t AES_KEY_SIZE = 32; // AES-256
};

SecurityManager::SecurityManager() : impl_(std::make_unique<Impl>()) {
    if (sodium_init() < 0) {
        LOG_ERROR("Failed to initialize libsodium");
    }
}

SecurityManager::~SecurityManager() noexcept {
    try {
        // Securely wipe ED25519 private key from memory
        if (!impl_->ed25519_private_key.empty()) {
            sodium_memzero(impl_->ed25519_private_key.data(),
                          impl_->ed25519_private_key.size());
            impl_->ed25519_private_key.clear();
        }
        
        // Securely wipe AES encryption key from memory
        if (!impl_->encryption_key.empty()) {
            sodium_memzero(impl_->encryption_key.data(),
                          impl_->encryption_key.size());
            impl_->encryption_key.clear();
        }
        
        // Free ECDH keypair
        if (impl_->ecdh_keypair) {
            EVP_PKEY_free(impl_->ecdh_keypair);
            impl_->ecdh_keypair = nullptr;
        }
    } catch (const std::exception& e) {
        // Cannot propagate exception from destructor
        // Still attempt to free ECDH keypair if exception occurred
        if (impl_->ecdh_keypair) {
            EVP_PKEY_free(impl_->ecdh_keypair);
            impl_->ecdh_keypair = nullptr;
        }
    } catch (...) {
        // Suppress all exceptions in destructor
        if (impl_->ecdh_keypair) {
            EVP_PKEY_free(impl_->ecdh_keypair);
            impl_->ecdh_keypair = nullptr;
        }
    }
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

void SecurityManager::SetCompressionManager(std::shared_ptr<CompressionManager> compression_manager) {
    impl_->compression_manager = compression_manager;
}

bool SecurityManager::EncryptPacket(const uint8_t* data, size_t size, std::vector<uint8_t>& encrypted_out) {
    std::vector<uint8_t> processed_data;

    // Step 1: Compress data if compression is enabled and available
    if (impl_->compression_manager) {
        std::vector<uint8_t> temp_data(data, data + size);
        std::vector<uint8_t> compressed_data = impl_->compression_manager->Compress(temp_data);
        if (!compressed_data.empty()) {
            processed_data = std::move(compressed_data);
            LOG_DEBUG("Compressed packet (" + std::to_string(size) + " -> " +
                     std::to_string(processed_data.size()) + " bytes)");
        } else {
            // Compression failed, use original data
            processed_data = temp_data;
            LOG_WARN("Compression failed, using original data");
        }
    } else {
        processed_data.assign(data, data + size);
    }

    // Step 2: Encrypt data if encryption is enabled
    if (!impl_->encryption_enabled) {
        encrypted_out = std::move(processed_data);
        return true;
    }

    if (!impl_->initialized || impl_->encryption_key.empty()) {
        LOG_ERROR("SecurityManager not initialized or no encryption key");
        return false;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        LOG_ERROR("Failed to create encryption context");
        return false;
    }

    try {
        // Generate random IV
        std::vector<uint8_t> iv(Impl::IV_SIZE);
        if (RAND_bytes(reinterpret_cast<unsigned char*>(&iv[0]), Impl::IV_SIZE) != 1) {
            LOG_ERROR("Failed to generate IV");
            EVP_CIPHER_CTX_free(ctx);
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
        encrypted_out.resize(Impl::IV_SIZE + processed_data.size() + Impl::TAG_SIZE);

        // Copy IV to output
        std::memcpy(encrypted_out.data(), iv.data(), Impl::IV_SIZE);

        // Encrypt data
        int len = 0;
        if (EVP_EncryptUpdate(ctx, encrypted_out.data() + Impl::IV_SIZE, &len,
                             processed_data.data(), static_cast<int>(processed_data.size())) != 1) {
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

        EVP_CIPHER_CTX_free(ctx);
        LOG_DEBUG("Encrypted packet (" + std::to_string(processed_data.size()) + " -> " +
                 std::to_string(encrypted_out.size()) + " bytes)");
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Encryption exception: " + std::string(e.what()));
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
}

bool SecurityManager::DecryptPacket(const uint8_t* data, size_t size, std::vector<uint8_t>& decrypted_out) {
    // Step 1: Decrypt data if encryption is enabled
    std::vector<uint8_t> intermediate_data;

    if (impl_->encryption_enabled) {
        if (!impl_->initialized || impl_->encryption_key.empty()) {
            LOG_ERROR("SecurityManager not initialized or no encryption key");
            return false;
        }

        if (size < Impl::IV_SIZE + Impl::TAG_SIZE) {
            LOG_ERROR("Packet too small for encrypted data");
            return false;
        }

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            LOG_ERROR("Failed to create decryption context");
            return false;
        }

        try {
            // Extract IV, ciphertext, and tag
            const uint8_t* iv = data;
            const uint8_t* ciphertext = data + Impl::IV_SIZE;
            const uint8_t* tag = data + size - Impl::TAG_SIZE;
            size_t ciphertext_len = size - Impl::IV_SIZE - Impl::TAG_SIZE;

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
            intermediate_data.resize(ciphertext_len);

            // Decrypt data
            int len = 0;
            if (EVP_DecryptUpdate(ctx, intermediate_data.data(), &len, ciphertext, static_cast<int>(ciphertext_len)) != 1) {
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
            if (EVP_DecryptFinal_ex(ctx, intermediate_data.data() + len, &len) != 1) {
                LOG_ERROR("Decryption finalization failed - authentication tag mismatch");
                EVP_CIPHER_CTX_free(ctx);
                return false;
            }
            plaintext_len += len;

            // Resize to actual plaintext size
            intermediate_data.resize(plaintext_len);

            EVP_CIPHER_CTX_free(ctx);
            LOG_DEBUG("Decrypted packet (" + std::to_string(size) + " -> " +
                     std::to_string(intermediate_data.size()) + " bytes)");

        } catch (const std::exception& e) {
            LOG_ERROR("Decryption exception: " + std::string(e.what()));
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
    } else {
        // No encryption, use data as-is
        intermediate_data.assign(data, data + size);
    }

    // Step 2: Decompress data if compression is enabled and available
    if (impl_->compression_manager) {
        std::vector<uint8_t> decompressed_data = impl_->compression_manager->Decompress(intermediate_data);
        if (!decompressed_data.empty()) {
            decrypted_out = std::move(decompressed_data);
            LOG_DEBUG("Decompressed packet (" + std::to_string(intermediate_data.size()) + " -> " +
                     std::to_string(decrypted_out.size()) + " bytes)");
            return true;
        } else {
            // Decompression failed, use intermediate data
            LOG_WARN("Decompression failed, using intermediate data");
            decrypted_out = std::move(intermediate_data);
            return true;
        }
    } else {
        // No compression, use intermediate data
        decrypted_out = std::move(intermediate_data);
        return true;
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

    // If signature checking is enabled, verify ED25519 signature (last 64 bytes)
    if (impl_->signature_enabled && size > Impl::ED25519_SIG_SIZE) {
        size_t payload_size = size - Impl::ED25519_SIG_SIZE;
        const uint8_t* payload = data;
        const uint8_t* signature = data + payload_size;

        std::lock_guard<std::mutex> lock(impl_->ed25519_mutex);
        if (impl_->ed25519_public_key.size() != Impl::ED25519_PUBKEY_SIZE) {
            LOG_ERROR("ED25519 public key not loaded");
            return false;
        }
        int result = crypto_sign_verify_detached(signature, payload, payload_size, impl_->ed25519_public_key.data());
        if (result != 0) {
            LOG_ERROR("ED25519 signature verification failed");
            return false;
        }
        LOG_DEBUG("ED25519 signature verified for packet (" + std::to_string(payload_size) + " bytes)");
    }

    LOG_DEBUG("Packet validated: type=0x" + std::to_string(packet_type) + ", size=" + std::to_string(size));
    return true;
}

bool SecurityManager::IsEncryptionEnabled() const {
    return impl_->encryption_enabled;
}

bool SecurityManager::LoadED25519Key(const std::string& key_path) {
    std::lock_guard<std::mutex> lock(impl_->ed25519_mutex);
    try {
        std::ifstream key_file(key_path, std::ios::binary);
        if (!key_file.is_open()) {
            LOG_ERROR("Failed to open ED25519 key file: " + key_path);
            return false;
        }
        impl_->ed25519_private_key.resize(Impl::ED25519_PRIVKEY_SIZE);
        key_file.read(reinterpret_cast<char*>(impl_->ed25519_private_key.data()), Impl::ED25519_PRIVKEY_SIZE);
        if (key_file.gcount() != Impl::ED25519_PRIVKEY_SIZE) {
            LOG_ERROR("ED25519 key file size invalid: " + key_path);
            return false;
        }
        // Derive public key from private key
        impl_->ed25519_public_key.resize(Impl::ED25519_PUBKEY_SIZE);
        if (crypto_sign_ed25519_sk_to_pk(impl_->ed25519_public_key.data(), impl_->ed25519_private_key.data()) != 0) {
            LOG_ERROR("Failed to derive ED25519 public key from private key");
            return false;
        }
        LOG_INFO("Loaded ED25519 private key and derived public key from: " + key_path);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception loading ED25519 key: " + std::string(e.what()));
        return false;
    }
}

bool SecurityManager::SignPacketED25519(const uint8_t* data, size_t size, std::vector<uint8_t>& signature_out) {
    std::lock_guard<std::mutex> lock(impl_->ed25519_mutex);
    if (!impl_->signature_enabled || impl_->ed25519_private_key.size() != Impl::ED25519_PRIVKEY_SIZE) {
        LOG_ERROR("ED25519 signature not enabled or key not loaded");
        return false;
    }
    signature_out.resize(Impl::ED25519_SIG_SIZE);
    if (crypto_sign_detached(signature_out.data(), nullptr, data, size, impl_->ed25519_private_key.data()) == 0) {
        LOG_INFO("ED25519 signature generated for packet (" + std::to_string(size) + " bytes)");
        return true;
    } else {
        LOG_ERROR("ED25519 signature generation failed");
        return false;
    }
}

bool SecurityManager::IsSignatureEnabled() const {
    return impl_->signature_enabled;
}

// ECDHE Key Exchange Implementation

bool SecurityManager::GenerateECDHKeypair() {
    std::lock_guard<std::mutex> lock(impl_->ecdhe_mutex);
    
    // Free existing keypair if any
    if (impl_->ecdh_keypair) {
        EVP_PKEY_free(impl_->ecdh_keypair);
        impl_->ecdh_keypair = nullptr;
    }

    // Create context for key generation
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
    if (!pctx) {
        LOG_ERROR("Failed to create ECDH context: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        return false;
    }

    // Initialize key generation
    if (EVP_PKEY_keygen_init(pctx) <= 0) {
        LOG_ERROR("Failed to initialize ECDH keygen: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        EVP_PKEY_CTX_free(pctx);
        return false;
    }

    // Set curve to secp256r1 (NIST P-256)
    if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_X9_62_prime256v1) <= 0) {
        LOG_ERROR("Failed to set EC curve: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        EVP_PKEY_CTX_free(pctx);
        return false;
    }

    // Generate keypair
    if (EVP_PKEY_keygen(pctx, &impl_->ecdh_keypair) <= 0) {
        LOG_ERROR("Failed to generate ECDH keypair: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        EVP_PKEY_CTX_free(pctx);
        return false;
    }

    EVP_PKEY_CTX_free(pctx);
    LOG_INFO("Generated ECDHE keypair (secp256r1)");
    return true;
}

std::vector<uint8_t> SecurityManager::GetPublicKey() const {
    std::lock_guard<std::mutex> lock(impl_->ecdhe_mutex);
    
    if (!impl_->ecdh_keypair) {
        LOG_ERROR("ECDH keypair not generated");
        return {};
    }

    // Serialize public key to DER format
    unsigned char* der = nullptr;
    int der_len = i2d_PUBKEY(impl_->ecdh_keypair, &der);
    
    if (der_len <= 0) {
        LOG_ERROR("Failed to serialize public key: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        return {};
    }

    std::vector<uint8_t> pubkey(der, der + der_len);
    OPENSSL_free(der);
    
    LOG_INFO("Serialized public key (" + std::to_string(pubkey.size()) + " bytes)");
    return pubkey;
}

bool SecurityManager::DeriveSharedKey(const std::vector<uint8_t>& peer_public_key) {
    std::lock_guard<std::mutex> lock(impl_->ecdhe_mutex);
    
    if (!impl_->ecdh_keypair) {
        LOG_ERROR("ECDH keypair not generated");
        return false;
    }

    if (peer_public_key.empty()) {
        LOG_ERROR("Peer public key is empty");
        return false;
    }

    // Deserialize peer's public key from DER format
    const unsigned char* der_data = peer_public_key.data();
    EVP_PKEY* peer_key = d2i_PUBKEY(nullptr, &der_data, peer_public_key.size());
    if (!peer_key) {
        LOG_ERROR("Failed to deserialize peer public key: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        return false;
    }

    // Create derivation context
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(impl_->ecdh_keypair, nullptr);
    if (!ctx) {
        LOG_ERROR("Failed to create derivation context: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        EVP_PKEY_free(peer_key);
        return false;
    }

    // Initialize key derivation
    if (EVP_PKEY_derive_init(ctx) <= 0) {
        LOG_ERROR("Failed to initialize key derivation: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(peer_key);
        return false;
    }

    // Set peer key
    if (EVP_PKEY_derive_set_peer(ctx, peer_key) <= 0) {
        LOG_ERROR("Failed to set peer key: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(peer_key);
        return false;
    }

    // Determine shared secret length
    size_t secret_len = 0;
    if (EVP_PKEY_derive(ctx, nullptr, &secret_len) <= 0) {
        LOG_ERROR("Failed to determine shared secret length: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(peer_key);
        return false;
    }

    // Derive shared secret
    std::vector<uint8_t> shared_secret(secret_len);
    if (EVP_PKEY_derive(ctx, shared_secret.data(), &secret_len) <= 0) {
        LOG_ERROR("Failed to derive shared secret: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(peer_key);
        return false;
    }

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(peer_key);

    LOG_INFO("Derived shared secret (" + std::to_string(secret_len) + " bytes)");

    // Derive AES-256 key from shared secret using HKDF-SHA256
    // Salt and info for domain separation
    const unsigned char salt[] = "P2P-ECDHE-Salt-v1";
    const unsigned char info[] = "P2P-AES256-Key-v1";

    EVP_PKEY_CTX* kdf_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr);
    if (!kdf_ctx) {
        LOG_ERROR("Failed to create HKDF context: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        return false;
    }

    if (EVP_PKEY_derive_init(kdf_ctx) <= 0) {
        LOG_ERROR("Failed to initialize HKDF: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        EVP_PKEY_CTX_free(kdf_ctx);
        return false;
    }

    if (EVP_PKEY_CTX_set_hkdf_md(kdf_ctx, EVP_sha256()) <= 0) {
        LOG_ERROR("Failed to set HKDF hash: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        EVP_PKEY_CTX_free(kdf_ctx);
        return false;
    }

    if (EVP_PKEY_CTX_set1_hkdf_salt(kdf_ctx, salt, sizeof(salt) - 1) <= 0) {
        LOG_ERROR("Failed to set HKDF salt: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        EVP_PKEY_CTX_free(kdf_ctx);
        return false;
    }

    if (EVP_PKEY_CTX_set1_hkdf_key(kdf_ctx, shared_secret.data(), shared_secret.size()) <= 0) {
        LOG_ERROR("Failed to set HKDF key: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        EVP_PKEY_CTX_free(kdf_ctx);
        return false;
    }

    if (EVP_PKEY_CTX_add1_hkdf_info(kdf_ctx, info, sizeof(info) - 1) <= 0) {
        LOG_ERROR("Failed to set HKDF info: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        EVP_PKEY_CTX_free(kdf_ctx);
        return false;
    }

    // Derive AES-256 key (32 bytes)
    impl_->encryption_key.resize(Impl::AES_KEY_SIZE);
    size_t key_len = Impl::AES_KEY_SIZE;
    if (EVP_PKEY_derive(kdf_ctx, impl_->encryption_key.data(), &key_len) <= 0) {
        LOG_ERROR("Failed to derive AES key: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        EVP_PKEY_CTX_free(kdf_ctx);
        return false;
    }

    EVP_PKEY_CTX_free(kdf_ctx);

    impl_->key_derived = true;
    impl_->encryption_enabled = true;
    
    LOG_INFO("Derived AES-256 key from ECDHE shared secret (" + std::to_string(key_len) + " bytes)");
    return true;
}

bool SecurityManager::IsKeyReady() const {
    std::lock_guard<std::mutex> lock(impl_->ecdhe_mutex);
    return !impl_->encryption_key.empty() && (impl_->key_derived || impl_->initialized);
}

} // namespace P2P