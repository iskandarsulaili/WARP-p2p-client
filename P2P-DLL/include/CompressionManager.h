#pragma once

#include "Types.h"
#include <memory>
#include <vector>
#include <string>

namespace P2P {

/**
 * Compression Manager
 * 
 * Handles packet compression using zlib or lz4
 * Compresses packets before encryption and decompresses after decryption
 */
class CompressionManager {
public:
    /**
     * Get singleton instance
     */
    static CompressionManager& GetInstance();

    /**
     * Initialize compression manager
     * @param config Configuration for compression settings
     * @return true if successful, false otherwise
     */
    bool Initialize(const CompressionConfig& config);

    /**
     * Compress packet data
     * @param data Input data to compress
     * @return Compressed data, empty if compression failed
     */
    std::vector<uint8_t> Compress(const std::vector<uint8_t>& data);

    /**
     * Decompress packet data
     * @param data Compressed data to decompress
     * @return Decompressed data, empty if decompression failed
     */
    std::vector<uint8_t> Decompress(const std::vector<uint8_t>& data);

    /**
     * Check if compression is enabled
     * @return true if compression is enabled
     */
    bool IsEnabled() const;

    /**
     * Get compression ratio statistics
     * @return Compression ratio (original_size / compressed_size)
     */
    double GetCompressionRatio() const;

    /**
     * Get compression algorithm name
     * @return Name of the compression algorithm
     */
    std::string GetAlgorithmName() const;

    CompressionManager();
    ~CompressionManager();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace P2P