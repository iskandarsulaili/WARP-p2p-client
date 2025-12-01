#include "../../include/CompressionManager.h"
#include "../../include/Logger.h"
#include <zlib.h>
#include <vector>
#include <memory>
#include <string>
#include <sstream>

namespace P2P {

struct CompressionManager::Impl {
    bool initialized = false;
    bool enabled = true;
    int compression_level = 6;
    
    // Statistics
    size_t total_bytes_compressed = 0;
    size_t total_bytes_decompressed = 0;
    size_t total_compressed_bytes = 0;
    size_t total_operations = 0;
    size_t failed_operations = 0;
};

CompressionManager::CompressionManager() : impl_(std::make_unique<Impl>()) {
    LOG_DEBUG("CompressionManager created");
}

CompressionManager::~CompressionManager() {
    Shutdown();
}

bool CompressionManager::Initialize(bool enabled, int compression_level) {
    if (impl_->initialized) {
        LOG_WARN("CompressionManager already initialized");
        return true;
    }

    impl_->enabled = enabled;
    impl_->compression_level = compression_level;
    impl_->initialized = true;

    LOG_INFO("CompressionManager initialized (enabled: " + std::string(enabled ? "true" : "false") + 
             ", level: " + std::to_string(compression_level) + ")");
    return true;
}

void CompressionManager::Shutdown() {
    if (impl_->initialized) {
        LOG_DEBUG("CompressionManager shutdown");
        impl_->initialized = false;
    }
}

bool CompressionManager::IsEnabled() const {
    return impl_->enabled;
}

bool CompressionManager::IsInitialized() const {
    return impl_->initialized;
}

bool CompressionManager::CompressPacket(const Packet& packet, std::vector<uint8_t>& compressed_data) {
    if (!impl_->initialized || !impl_->enabled) {
        return false;
    }

    try {
        // Convert packet to byte array for compression
        std::vector<uint8_t> packet_data;
        packet_data.reserve(sizeof(Packet) + packet.length);
        
        // Add packet header
        const uint8_t* header_ptr = reinterpret_cast<const uint8_t*>(&packet);
        packet_data.insert(packet_data.end(), header_ptr, header_ptr + sizeof(Packet));
        
        // Add packet payload
        if (packet.length > 0 && packet.data != nullptr) {
            packet_data.insert(packet_data.end(), packet.data, packet.data + packet.length);
        }

        return CompressData(packet_data, compressed_data);
    } catch (const std::exception& e) {
        LOG_ERROR("Error compressing packet: " + std::string(e.what()));
        impl_->failed_operations++;
        return false;
    }
}

bool CompressionManager::DecompressPacket(const std::vector<uint8_t>& compressed_data, Packet& decompressed_packet) {
    if (!impl_->initialized || !impl_->enabled) {
        return false;
    }

    try {
        std::vector<uint8_t> decompressed_data;
        if (!DecompressData(compressed_data, decompressed_data)) {
            return false;
        }

        // Check if we have enough data for at least the packet header
        if (decompressed_data.size() < sizeof(Packet)) {
            LOG_ERROR("Decompressed data too small for packet header");
            impl_->failed_operations++;
            return false;
        }

        // Copy packet header
        memcpy(&decompressed_packet, decompressed_data.data(), sizeof(Packet));

        // Copy packet payload if present
        if (decompressed_packet.length > 0) {
            size_t payload_size = decompressed_data.size() - sizeof(Packet);
            if (payload_size < decompressed_packet.length) {
                LOG_ERROR("Decompressed payload size mismatch");
                impl_->failed_operations++;
                return false;
            }

            // Allocate memory for payload and copy
            if (decompressed_packet.data != nullptr) {
                delete[] decompressed_packet.data;
            }
            decompressed_packet.data = new uint8_t[decompressed_packet.length];
            memcpy(decompressed_packet.data, decompressed_data.data() + sizeof(Packet), decompressed_packet.length);
        }

        impl_->total_operations++;
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Error decompressing packet: " + std::string(e.what()));
        impl_->failed_operations++;
        return false;
    }
}

bool CompressionManager::CompressData(const std::vector<uint8_t>& input_data, std::vector<uint8_t>& compressed_data) {
    if (!impl_->initialized || !impl_->enabled || input_data.empty()) {
        compressed_data = input_data;
        return true;
    }

    try {
        // Estimate compressed size (zlib recommends sourceLen + 0.1% + 12 bytes)
        uLongf compressed_size = compressBound(input_data.size());
        compressed_data.resize(compressed_size);

        int result = compress2(compressed_data.data(), &compressed_size,
                             input_data.data(), input_data.size(),
                             impl_->compression_level);

        if (result == Z_OK) {
            compressed_data.resize(compressed_size);
            
            // Update statistics
            impl_->total_bytes_compressed += input_data.size();
            impl_->total_compressed_bytes += compressed_size;
            impl_->total_operations++;
            
            LOG_DEBUG("Compressed " + std::to_string(input_data.size()) + " bytes to " + 
                     std::to_string(compressed_size) + " bytes (ratio: " +
                     std::to_string(static_cast<float>(input_data.size()) / compressed_size) + ")");
            return true;
        } else {
            LOG_ERROR("Compression failed with error code: " + std::to_string(result));
            impl_->failed_operations++;
            return false;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error in compression: " + std::string(e.what()));
        impl_->failed_operations++;
        return false;
    }
}

bool CompressionManager::DecompressData(const std::vector<uint8_t>& compressed_data, std::vector<uint8_t>& decompressed_data) {
    if (!impl_->initialized || !impl_->enabled || compressed_data.empty()) {
        decompressed_data = compressed_data;
        return true;
    }

    try {
        // Start with a reasonable buffer size
        const size_t INITIAL_BUFFER_SIZE = compressed_data.size() * 4;
        decompressed_data.resize(INITIAL_BUFFER_SIZE);
        
        uLongf decompressed_size = decompressed_data.size();
        int result = Z_BUF_ERROR;
        int attempts = 0;
        const int MAX_ATTEMPTS = 3;

        while (result == Z_BUF_ERROR && attempts < MAX_ATTEMPTS) {
            result = uncompress(decompressed_data.data(), &decompressed_size,
                               compressed_data.data(), compressed_data.size());

            if (result == Z_BUF_ERROR) {
                // Buffer too small, double the size and try again
                decompressed_size *= 2;
                decompressed_data.resize(decompressed_size);
                attempts++;
            }
        }

        if (result == Z_OK) {
            decompressed_data.resize(decompressed_size);
            
            // Update statistics
            impl_->total_bytes_decompressed += decompressed_size;
            impl_->total_operations++;
            
            LOG_DEBUG("Decompressed " + std::to_string(compressed_data.size()) + " bytes to " + 
                     std::to_string(decompressed_size) + " bytes");
            return true;
        } else {
            LOG_ERROR("Decompression failed with error code: " + std::to_string(result));
            impl_->failed_operations++;
            return false;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error in decompression: " + std::string(e.what()));
        impl_->failed_operations++;
        return false;
    }
}

float CompressionManager::GetCompressionRatio() const {
    if (impl_->total_compressed_bytes == 0) {
        return 1.0f;
    }
    return static_cast<float>(impl_->total_bytes_compressed) / impl_->total_compressed_bytes;
}

int CompressionManager::GetCompressionLevel() const {
    return impl_->compression_level;
}

void CompressionManager::SetCompressionLevel(int level) {
    if (level >= 0 && level <= 9) {
        impl_->compression_level = level;
        LOG_INFO("Compression level set to: " + std::to_string(level));
    } else {
        LOG_WARN("Invalid compression level: " + std::to_string(level) + ", must be between 0-9");
    }
}

void CompressionManager::SetEnabled(bool enabled) {
    impl_->enabled = enabled;
    LOG_INFO("Compression " + std::string(enabled ? "enabled" : "disabled"));
}

std::string CompressionManager::GetStats() const {
    std::stringstream ss;
    ss << "Compression Statistics:\n";
    ss << "  Enabled: " << (impl_->enabled ? "Yes" : "No") << "\n";
    ss << "  Level: " << impl_->compression_level << "\n";
    ss << "  Total operations: " << impl_->total_operations << "\n";
    ss << "  Failed operations: " << impl_->failed_operations << "\n";
    ss << "  Total bytes compressed: " << impl_->total_bytes_compressed << "\n";
    ss << "  Total bytes decompressed: " << impl_->total_bytes_decompressed << "\n";
    ss << "  Total compressed bytes: " << impl_->total_compressed_bytes << "\n";
    ss << "  Overall compression ratio: " << GetCompressionRatio() << "\n";
    return ss.str();
}

} // namespace P2P