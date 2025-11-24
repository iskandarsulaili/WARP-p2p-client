// test_ecdhe_key_exchange.cpp - Unit test for ECDHE key exchange
#include "../include/SecurityManager.h"
#include "../include/Logger.h"
#include <iostream>
#include <iomanip>
#include <cassert>

using namespace P2P;

// Helper function to print hex bytes
void print_hex(const std::string& label, const std::vector<uint8_t>& data) {
    std::cout << label << " (" << data.size() << " bytes): ";
    for (size_t i = 0; i < std::min(data.size(), size_t(32)); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(data[i]);
    }
    if (data.size() > 32) {
        std::cout << "...";
    }
    std::cout << std::dec << std::endl;
}

// Helper function to get encryption key from SecurityManager (for testing)
// Note: This accesses private implementation details for testing purposes only
std::vector<uint8_t> get_encryption_key(SecurityManager& sm) {
    // In production, this would need a friend declaration or getter method
    // For now, we'll test by encrypting/decrypting the same message
    return {};
}

int main() {
    std::cout << "=== ECDHE Key Exchange Unit Test ===" << std::endl << std::endl;

    // Test 1: Basic keypair generation
    std::cout << "Test 1: Generate ECDHE keypairs for two peers..." << std::endl;
    
    SecurityManager peer_a;
    SecurityManager peer_b;
    
    peer_a.Initialize(true);
    peer_b.Initialize(true);
    
    assert(peer_a.GenerateECDHKeypair() && "Peer A keypair generation failed");
    assert(peer_b.GenerateECDHKeypair() && "Peer B keypair generation failed");
    
    std::cout << "✓ Both peers generated ECDHE keypairs successfully" << std::endl << std::endl;

    // Test 2: Public key serialization
    std::cout << "Test 2: Serialize public keys..." << std::endl;
    
    std::vector<uint8_t> pubkey_a = peer_a.GetPublicKey();
    std::vector<uint8_t> pubkey_b = peer_b.GetPublicKey();
    
    assert(!pubkey_a.empty() && "Peer A public key is empty");
    assert(!pubkey_b.empty() && "Peer B public key is empty");
    
    print_hex("Peer A public key", pubkey_a);
    print_hex("Peer B public key", pubkey_b);
    
    std::cout << "✓ Both public keys serialized successfully" << std::endl << std::endl;

    // Test 3: Key exchange - derive shared secret
    std::cout << "Test 3: Perform key exchange (derive shared secrets)..." << std::endl;
    
    // Peer A derives key using Peer B's public key
    assert(peer_a.DeriveSharedKey(pubkey_b) && "Peer A failed to derive shared key");
    
    // Peer B derives key using Peer A's public key
    assert(peer_b.DeriveSharedKey(pubkey_a) && "Peer B failed to derive shared key");
    
    std::cout << "✓ Both peers derived shared keys successfully" << std::endl << std::endl;

    // Test 4: Verify encryption readiness
    std::cout << "Test 4: Verify encryption readiness..." << std::endl;
    
    assert(peer_a.IsKeyReady() && "Peer A key not ready");
    assert(peer_b.IsKeyReady() && "Peer B key not ready");
    assert(peer_a.IsEncryptionEnabled() && "Peer A encryption not enabled");
    assert(peer_b.IsEncryptionEnabled() && "Peer B encryption not enabled");
    
    std::cout << "✓ Both peers have encryption keys ready" << std::endl << std::endl;

    // Test 5: Verify identical keys by encrypt/decrypt test
    std::cout << "Test 5: Verify keys are identical (encrypt/decrypt test)..." << std::endl;
    
    // Test message
    const char* test_message = "Hello, P2P World! This is a test of ECDHE key exchange.";
    size_t msg_len = strlen(test_message);
    
    // Peer A encrypts
    std::vector<uint8_t> encrypted;
    assert(peer_a.EncryptPacket(reinterpret_cast<const uint8_t*>(test_message), 
                                 msg_len, encrypted) && 
           "Peer A encryption failed");
    
    std::cout << "Original message: " << test_message << std::endl;
    std::cout << "Encrypted size: " << encrypted.size() << " bytes" << std::endl;
    
    // Peer B decrypts
    std::vector<uint8_t> decrypted;
    assert(peer_b.DecryptPacket(encrypted.data(), encrypted.size(), decrypted) && 
           "Peer B decryption failed");
    
    std::cout << "Decrypted size: " << decrypted.size() << " bytes" << std::endl;
    
    // Verify decrypted message matches original
    assert(decrypted.size() == msg_len && "Decrypted size mismatch");
    assert(memcmp(decrypted.data(), test_message, msg_len) == 0 && 
           "Decrypted content mismatch");
    
    std::string decrypted_str(reinterpret_cast<char*>(decrypted.data()), decrypted.size());
    std::cout << "Decrypted message: " << decrypted_str << std::endl;
    
    std::cout << "✓ Encryption/decryption successful - Keys are identical!" << std::endl << std::endl;

    // Test 6: Reverse direction (B encrypts, A decrypts)
    std::cout << "Test 6: Verify reverse direction (B encrypts, A decrypts)..." << std::endl;
    
    const char* reverse_message = "Testing reverse encryption direction!";
    size_t reverse_len = strlen(reverse_message);
    
    std::vector<uint8_t> encrypted_reverse;
    assert(peer_b.EncryptPacket(reinterpret_cast<const uint8_t*>(reverse_message), 
                                 reverse_len, encrypted_reverse) && 
           "Peer B encryption failed");
    
    std::vector<uint8_t> decrypted_reverse;
    assert(peer_a.DecryptPacket(encrypted_reverse.data(), encrypted_reverse.size(), 
                                 decrypted_reverse) && 
           "Peer A decryption failed");
    
    assert(decrypted_reverse.size() == reverse_len && "Reverse decrypted size mismatch");
    assert(memcmp(decrypted_reverse.data(), reverse_message, reverse_len) == 0 && 
           "Reverse decrypted content mismatch");
    
    std::cout << "✓ Reverse encryption/decryption successful!" << std::endl << std::endl;

    // Test 7: Multiple messages
    std::cout << "Test 7: Verify multiple sequential messages..." << std::endl;
    
    for (int i = 0; i < 5; ++i) {
        std::string msg = "Message #" + std::to_string(i) + " - Testing sequential encryption";
        
        std::vector<uint8_t> enc;
        assert(peer_a.EncryptPacket(reinterpret_cast<const uint8_t*>(msg.c_str()), 
                                     msg.size(), enc) && 
               "Sequential encryption failed");
        
        std::vector<uint8_t> dec;
        assert(peer_b.DecryptPacket(enc.data(), enc.size(), dec) && 
               "Sequential decryption failed");
        
        assert(dec.size() == msg.size() && "Sequential size mismatch");
        assert(memcmp(dec.data(), msg.c_str(), msg.size()) == 0 && 
               "Sequential content mismatch");
    }
    
    std::cout << "✓ All sequential messages encrypted/decrypted successfully!" << std::endl << std::endl;

    // Test 8: Error cases
    std::cout << "Test 8: Test error handling..." << std::endl;
    
    // Test with invalid peer public key
    SecurityManager peer_c;
    peer_c.Initialize(true);
    peer_c.GenerateECDHKeypair();
    
    std::vector<uint8_t> invalid_key = {0x01, 0x02, 0x03}; // Too small, invalid
    assert(!peer_c.DeriveSharedKey(invalid_key) && 
           "Should fail with invalid public key");
    
    std::cout << "✓ Error handling works correctly" << std::endl << std::endl;

    // Summary
    std::cout << "=== ALL TESTS PASSED ===" << std::endl;
    std::cout << std::endl;
    std::cout << "Summary:" << std::endl;
    std::cout << "- ECDHE keypair generation: ✓" << std::endl;
    std::cout << "- Public key serialization: ✓" << std::endl;
    std::cout << "- Shared secret derivation: ✓" << std::endl;
    std::cout << "- Identical key derivation: ✓" << std::endl;
    std::cout << "- Bidirectional encryption: ✓" << std::endl;
    std::cout << "- Sequential messages: ✓" << std::endl;
    std::cout << "- Error handling: ✓" << std::endl;
    std::cout << std::endl;
    std::cout << "ECDHE key exchange implementation is WORKING CORRECTLY!" << std::endl;

    return 0;
}