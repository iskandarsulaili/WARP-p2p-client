#pragma once

#include <vector>
#include <cstdint>
#include <memory>
#include <string>
#include "../Types.h"

namespace P2P {

/**
 * Compression statistics for monitoring
 */
struct CompressionStats {
    uint64_t total_packets_compressed = 0;
    uint64_t total_packets_decompressed = 0;
    uint64_t total_bytes_before_compression = 0;
    uint64_t total_bytes_after_compression = 0;
    uint64_t compression_failures = 0;
    uint64_t decompression_failures = 0;
    float average_compression_ratio = 0.0f;
    
    // Update compression ratio
    void UpdateRatio() {
        if (total_bytes_before_compression > 0) {
            average_compression_ratio = static_cast<float>(total_bytes_after_compression) / 
                                       static_cast<float>(total_bytes_before_compression);
        }
    }
};

/**
 * Compression Manager for packet compression/decompression
 * Uses zlib for DEFLATE compression algorithm
 */
class CompressionManager {
public:
    CompressionManager();
    ~CompressionManager();

    /**
     * Initialize compression manager with configuration
     */
    bool Initialize(const CompressionConfig& config);

    /**
     * Compress data using zlib DEFLATE
     * @param input_data Raw data to compress
     * @param compressed_data Output compressed data
     * @return true if compression successful
     */
    bool CompressData(const std::vector<uint8_t>& input_data, std::vector<uint8_t>& compressed_data);

    /**
     * Decompress data using zlib INFLATE
     * @param compressed_data Compressed data to decompress
     * @param decompressed_data Output decompressed data
     * @return true if decompression successful
     */
    bool DecompressData(const std::vector<uint8_t>& compressed_data, std::vector<uint8_t>& decompressed_data);

    /**
     * Check if compression should be attempted for this packet
     * Based on size thresholds and compression ratio expectations
     */
    bool ShouldCompress(const std::vector<uint8_t>& data) const;

    /**
     * Get current compression statistics
     */
    const CompressionStats& GetStats() const { return stats_; }

    /**
     * Reset compression statistics
     */
    void ResetStats();

    /**
     * Get compression configuration
     */
    const CompressionConfig& GetConfig() const { return config_; }

    /**
     * Update compression configuration
     */
    void UpdateConfig(const CompressionConfig& new_config);

private:
    CompressionConfig config_;
    CompressionStats stats_;
    
    // Private implementation to avoid zlib header exposure
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace P2P