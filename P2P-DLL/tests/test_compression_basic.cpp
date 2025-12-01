#include "../../include/CompressionManager.h"
#include "../../include/Logger.h"
#include <iostream>
#include <vector>

int main() {
    // Initialize logger
    Logger::Initialize("test_compression.log", LogLevel::DEBUG);
    
    P2P::CompressionManager comp_manager;
    
    if (!comp_manager.Initialize(true, 6)) {
        std::cerr << "Failed to initialize CompressionManager" << std::endl;
        return 1;
    }
    
    // Test data - typical game packet data
    std::vector<uint8_t> test_data;
    for (int i = 0; i < 1000; i++) {
        test_data.push_back(static_cast<uint8_t>(i % 256));
    }
    
    std::cout << "Original data size: " << test_data.size() << " bytes" << std::endl;
    
    // Test compression
    std::vector<uint8_t> compressed;
    if (comp_manager.CompressData(test_data, compressed)) {
        std::cout << "Compressed size: " << compressed.size() << " bytes" << std::endl;
        std::cout << "Compression ratio: " << (static_cast<float>(test_data.size()) / compressed.size()) << std::endl;
    } else {
        std::cerr << "Compression failed" << std::endl;
        return 1;
    }
    
    // Test decompression
    std::vector<uint8_t> decompressed;
    if (comp_manager.DecompressData(compressed, decompressed)) {
        std::cout << "Decompressed size: " << decompressed.size() << " bytes" << std::endl;
        
        // Verify data integrity
        if (test_data.size() != decompressed.size()) {
            std::cerr << "Size mismatch after decompression" << std::endl;
            return 1;
        }
        
        for (size_t i = 0; i < test_data.size(); i++) {
            if (test_data[i] != decompressed[i]) {
                std::cerr << "Data mismatch at position " << i << std::endl;
                return 1;
            }
        }
        
        std::cout << "Compression test passed successfully!" << std::endl;
    } else {
        std::cerr << "Decompression failed" << std::endl;
        return 1;
    }
    
    comp_manager.Shutdown();
    Logger::Shutdown();
    return 0;
}