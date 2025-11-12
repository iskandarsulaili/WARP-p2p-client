#include "../../include/CompressionManager.h"
#include "../../include/ConfigManager.h"
#include "../../include/Logger.h"
#include <zlib.h>
#include <lz4.h>
#include <lz4hc.h>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

namespace P2P {

struct CompressionManager::Impl {
    bool enabled = false;
    bool use_lz4 = true;
    int compression_level = 6;
    
    // Statistics
    size_t total_original = 0;
    size_t total_compressed = 0;
    size_t compression_count = 0;
    
    // LZ4 context
    LZ4_stream_t* lz4_stream = nullptr;
    LZ4_streamHC_t* lz4hc_stream = nullptr;
};

CompressionManager::CompressionManager() : impl_(std::make_unique<Impl>()) {
    LOG_DEBUG("CompressionManager created");
}

CompressionManager::~CompressionManager() {
    if (impl_->lz4_stream) {
        LZ4_freeStream(impl_->lz4_stream);
    }
    if (impl_->lz4hc_stream) {
        LZ4_freeStreamHC(impl_->lz4hc_stream);
    }
    LOG_DEBUG("CompressionManager destroyed");
}

CompressionManager& CompressionManager::GetInstance() {
    static CompressionManager instance;
    return instance;
}

bool CompressionManager::Initialize(const CompressionConfig& config) {
    impl_->enabled = config.enabled;
    impl_->use_lz4 = config.algorithm == "lz4";
    impl_->compression_level = config.compression_level;
    
    if (!impl_->enabled) {
        LOG_INFO("Compression is disabled");
        return true;
    }
    
    // Initialize LZ4 streams if using LZ4
    if (impl_->use_lz4) {
        if (impl_->compression_level > 9) {
            impl_->lz4hc_stream = LZ4_createStreamHC();
            if (!impl_->lz4hc_stream) {
                LOG_ERROR("Failed to create LZ4 HC stream");
                impl_->enabled = false;
                return false;
            }
        } else {
            impl_->lz4_stream = LZ4_createStream();
            if (!impl_->lz4_stream) {
                LOG_ERROR("Failed to create LZ4 stream");
                impl_->enabled = false;
                return false;
            }
        }
        std::ostringstream oss;
        oss << "LZ4 compression initialized (level: " << impl_->compression_level << ")";
        LOG_INFO(oss.str());
    } else {
        std::ostringstream oss;
        oss << "Zlib compression initialized (level: " << impl_->compression_level << ")";
        LOG_INFO(oss.str());
    }
    
    return true;
}

std::vector<uint8_t> CompressionManager::Compress(const std::vector<uint8_t>& data) {
    if (!impl_->enabled || data.empty()) {
        return data;
    }
    
    std::vector<uint8_t> compressed;
    
    if (impl_->use_lz4) {
        // LZ4 compression
        const int max_compressed_size = LZ4_compressBound(static_cast<int>(data.size()));
        compressed.resize(max_compressed_size);
        
        int compressed_size;
        if (impl_->compression_level > 9) {
            compressed_size = LZ4_compress_HC(
                reinterpret_cast<const char*>(data.data()),
                reinterpret_cast<char*>(compressed.data()),
                static_cast<int>(data.size()),
                max_compressed_size,
                impl_->compression_level
            );
        } else {
            compressed_size = LZ4_compress_default(
                reinterpret_cast<const char*>(data.data()),
                reinterpret_cast<char*>(compressed.data()),
                static_cast<int>(data.size()),
                max_compressed_size
            );
        }
        
        if (compressed_size <= 0) {
            LOG_ERROR("LZ4 compression failed");
            return data;
        }
        
        compressed.resize(compressed_size);
    } else {
        // Zlib compression
        uLongf compressed_size = compressBound(static_cast<uLong>(data.size()));
        compressed.resize(compressed_size);
        
        int result = compress2(
            compressed.data(),
            &compressed_size,
            data.data(),
            static_cast<uLong>(data.size()),
            impl_->compression_level
        );
        
        if (result != Z_OK) {
            std::ostringstream oss;
            oss << "Zlib compression failed: " << result;
            LOG_ERROR(oss.str());
            return data;
        }
        
        compressed.resize(compressed_size);
    }
    
    // Update statistics
    impl_->total_original += data.size();
    impl_->total_compressed += compressed.size();
    impl_->compression_count++;
    
    std::ostringstream oss;
    oss << "Compressed " << data.size() << " bytes to " << compressed.size() << " bytes (ratio: ";
    oss << std::fixed << std::setprecision(2) << (static_cast<double>(data.size()) / compressed.size()) << ")";
    LOG_DEBUG(oss.str());
    
    return compressed;
}

std::vector<uint8_t> CompressionManager::Decompress(const std::vector<uint8_t>& data) {
    if (!impl_->enabled || data.empty()) {
        return data;
    }
    
    // For now, we'll assume the original size is stored in the first 4 bytes
    // In a real implementation, we'd need a proper header format
    if (data.size() < 4) {
        LOG_ERROR("Compressed data too small for header");
        return data;
    }
    
    uint32_t original_size;
    std::memcpy(&original_size, data.data(), sizeof(original_size));
    
    std::vector<uint8_t> decompressed(original_size);
    
    if (impl_->use_lz4) {
        // LZ4 decompression
        int decompressed_size = LZ4_decompress_safe(
            reinterpret_cast<const char*>(data.data() + sizeof(original_size)),
            reinterpret_cast<char*>(decompressed.data()),
            static_cast<int>(data.size() - sizeof(original_size)),
            static_cast<int>(original_size)
        );
        
        if (decompressed_size != static_cast<int>(original_size)) {
            std::ostringstream oss;
            oss << "LZ4 decompression failed: expected " << original_size << ", got " << decompressed_size;
            LOG_ERROR(oss.str());
            return data;
        }
    } else {
        // Zlib decompression
        uLongf decompressed_size = original_size;
        int result = uncompress(
            decompressed.data(),
            &decompressed_size,
            data.data() + sizeof(original_size),
            static_cast<uLong>(data.size() - sizeof(original_size))
        );
        
        if (result != Z_OK || decompressed_size != original_size) {
            std::ostringstream oss;
            oss << "Zlib decompression failed: " << result;
            LOG_ERROR(oss.str());
            return data;
        }
    }
    
    return decompressed;
}

bool CompressionManager::IsEnabled() const {
    return impl_->enabled;
}

double CompressionManager::GetCompressionRatio() const {
    if (impl_->total_compressed == 0) {
        return 1.0;
    }
    return static_cast<double>(impl_->total_original) / impl_->total_compressed;
}

std::string CompressionManager::GetAlgorithmName() const {
    return impl_->use_lz4 ? "LZ4" : "Zlib";
}

} // namespace P2P