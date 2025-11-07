#ifndef P2P_SECURITY_MANAGER_HPP
#define P2P_SECURITY_MANAGER_HPP

#include "P2PEncryption.hpp"
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>
#include <map>
#include <openssl/ssl.h>
#include <openssl/x509.h>

/**
 * @brief P2P Security Manager
 * 
 * Comprehensive security management for P2P connections including:
 * - SSL/TLS certificate validation
 * - End-to-end encryption
 * - Anti-cheat integration points
 * - Security event monitoring
 * - Threat detection and mitigation
 * 
 * Thread Safety: All public methods are thread-safe
 * 
 * Usage:
 *   P2PSecurityManager security;
 *   security.initialize(config);
 *   security.validate_peer_certificate(peer_id, cert);
 *   security.encrypt_for_peer(peer_id, data);
 */
class P2PSecurityManager {
public:
    /**
     * @brief Security configuration
     */
    struct SecurityConfig {
        bool enable_encryption;              // Enable E2E encryption
        bool require_certificates;           // Require peer certificates
        bool enable_certificate_pinning;     // Enable certificate pinning
        bool enable_anti_cheat;              // Enable anti-cheat integration
        std::string ca_cert_path;            // Path to CA certificate
        std::string client_cert_path;        // Path to client certificate
        std::string client_key_path;         // Path to client private key
        uint32_t max_failed_validations;    // Max failed validations before ban
        uint32_t security_event_threshold;   // Security events before alert
    };
    
    /**
     * @brief Security event types
     */
    enum class SecurityEvent {
        CERTIFICATE_VALIDATION_FAILED,
        ENCRYPTION_FAILED,
        DECRYPTION_FAILED,
        INVALID_SIGNATURE,
        REPLAY_ATTACK_DETECTED,
        TAMPERING_DETECTED,
        SUSPICIOUS_BEHAVIOR,
        ANTI_CHEAT_VIOLATION
    };
    
    /**
     * @brief Peer security status
     */
    struct PeerSecurityStatus {
        std::string peer_id;
        bool certificate_valid;
        bool encryption_enabled;
        uint32_t failed_validations;
        uint32_t security_events;
        bool is_trusted;
        bool is_banned;
        uint64_t last_validation_time;
    };
    
    // Callback type definitions
    using SecurityEventCallback = std::function<void(const std::string& peer_id, SecurityEvent event, const std::string& details)>;
    using PeerBannedCallback = std::function<void(const std::string& peer_id, const std::string& reason)>;
    using AntiCheatCallback = std::function<bool(const std::string& peer_id, const std::vector<uint8_t>& data)>;
    
    /**
     * @brief Constructor
     */
    P2PSecurityManager();
    
    /**
     * @brief Destructor
     */
    ~P2PSecurityManager();
    
    // Disable copy
    P2PSecurityManager(const P2PSecurityManager&) = delete;
    P2PSecurityManager& operator=(const P2PSecurityManager&) = delete;
    
    /**
     * @brief Initialize security manager
     * @param config Security configuration
     * @return true if initialization successful
     */
    bool initialize(const SecurityConfig& config);
    
    /**
     * @brief Shutdown security manager
     */
    void shutdown();
    
    /**
     * @brief Validate peer SSL/TLS certificate
     * @param peer_id Peer ID
     * @param cert_data Certificate data (PEM format)
     * @return true if certificate is valid
     */
    bool validate_peer_certificate(const std::string& peer_id, const std::string& cert_data);
    
    /**
     * @brief Encrypt data for peer
     * @param peer_id Peer ID
     * @param plaintext Data to encrypt
     * @return Encrypted data or empty vector on failure
     */
    std::vector<uint8_t> encrypt_for_peer(const std::string& peer_id, const std::vector<uint8_t>& plaintext);
    
    /**
     * @brief Decrypt data from peer
     * @param peer_id Peer ID
     * @param ciphertext Encrypted data
     * @return Decrypted data or empty vector on failure
     */
    std::vector<uint8_t> decrypt_from_peer(const std::string& peer_id, const std::vector<uint8_t>& ciphertext);
    
    /**
     * @brief Perform anti-cheat validation on data
     * @param peer_id Peer ID
     * @param data Data to validate
     * @return true if data passes anti-cheat checks
     */
    bool validate_anti_cheat(const std::string& peer_id, const std::vector<uint8_t>& data);
    
    /**
     * @brief Check if peer is trusted
     * @param peer_id Peer ID
     * @return true if peer is trusted
     */
    bool is_peer_trusted(const std::string& peer_id) const;
    
    /**
     * @brief Check if peer is banned
     * @param peer_id Peer ID
     * @return true if peer is banned
     */
    bool is_peer_banned(const std::string& peer_id) const;
    
    /**
     * @brief Ban a peer
     * @param peer_id Peer ID
     * @param reason Ban reason
     */
    void ban_peer(const std::string& peer_id, const std::string& reason);
    
    /**
     * @brief Unban a peer
     * @param peer_id Peer ID
     */
    void unban_peer(const std::string& peer_id);
    
    /**
     * @brief Get peer security status
     * @param peer_id Peer ID
     * @return Peer security status or nullptr if not found
     */
    std::unique_ptr<PeerSecurityStatus> get_peer_status(const std::string& peer_id) const;
    
    /**
     * @brief Get all peer security statuses
     */
    std::vector<PeerSecurityStatus> get_all_peer_statuses() const;
    
    /**
     * @brief Report security event
     * @param peer_id Peer ID
     * @param event Security event type
     * @param details Event details
     */
    void report_security_event(const std::string& peer_id, SecurityEvent event, const std::string& details);
    
    // Event callbacks
    void on_security_event(SecurityEventCallback callback) { security_event_callback_ = callback; }
    void on_peer_banned(PeerBannedCallback callback) { peer_banned_callback_ = callback; }
    void on_anti_cheat_check(AntiCheatCallback callback) { anti_cheat_callback_ = callback; }

private:
    SecurityConfig config_;
    
    // Encryption managers per peer
    std::map<std::string, std::unique_ptr<P2PEncryption>> peer_encryption_;
    
    // Peer security tracking
    std::map<std::string, PeerSecurityStatus> peer_status_;
    std::map<std::string, std::string> banned_peers_;  // peer_id -> reason
    
    // SSL/TLS context
    SSL_CTX* ssl_ctx_;
    X509* ca_cert_;
    
    // Thread safety
    mutable std::mutex mutex_;
    
    // Callbacks
    SecurityEventCallback security_event_callback_;
    PeerBannedCallback peer_banned_callback_;
    AntiCheatCallback anti_cheat_callback_;
    
    // Internal methods
    bool load_ca_certificate(const std::string& path);
    bool verify_certificate_chain(X509* cert);
    bool check_certificate_pinning(const std::string& peer_id, X509* cert);
    
    P2PEncryption* get_or_create_peer_encryption(const std::string& peer_id);
    PeerSecurityStatus& get_or_create_peer_status(const std::string& peer_id);
    
    void increment_failed_validations(const std::string& peer_id);
    void check_ban_threshold(const std::string& peer_id);
    
    void log_info(const std::string& message);
    void log_error(const std::string& message);
    void log_security(const std::string& peer_id, const std::string& message);
};

#endif // P2P_SECURITY_MANAGER_HPP

