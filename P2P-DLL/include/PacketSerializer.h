#pragma once

#include "Types.h"
#include <vector>
#include <cstdint>
#include <cstring>

namespace P2P {

/**
 * @brief Packet serialization and deserialization
 * 
 * Binary format:
 * [Magic: 2 bytes] [Version: 1 byte] [Packet ID: 2 bytes] [Type: 2 bytes] 
 * [Length: 4 bytes] [Data: variable] [Checksum: 4 bytes]
 * 
 * Total header size: 15 bytes
 */
class PacketSerializer {
public:
    // Magic number to identify P2P packets
    static constexpr uint16_t MAGIC = 0x5032;  // "P2" in hex
    static constexpr uint8_t VERSION = 0x01;
    
    /**
     * @brief Serialize a packet to binary format
     * @param packet Packet to serialize
     * @param output Output buffer for serialized data
     * @return True if serialization succeeded
     */
    static bool Serialize(const Packet& packet, std::vector<uint8_t>& output);
    
    /**
     * @brief Deserialize binary data to a packet
     * @param data Input binary data
     * @param length Length of input data
     * @param packet Output packet
     * @return True if deserialization succeeded
     */
    static bool Deserialize(const uint8_t* data, size_t length, Packet& packet);
    
    /**
     * @brief Validate packet integrity
     * @param data Binary packet data
     * @param length Length of data
     * @return True if packet is valid
     */
    static bool Validate(const uint8_t* data, size_t length);
    
    /**
     * @brief Get maximum packet size
     * @return Maximum packet size in bytes
     */
    static constexpr size_t GetMaxPacketSize() { return 65535; }
    
    /**
     * @brief Get header size
     * @return Header size in bytes
     */
    static constexpr size_t GetHeaderSize() { return 15; }

private:
    /**
     * @brief Calculate CRC32 checksum
     * @param data Data to checksum
     * @param length Length of data
     * @return CRC32 checksum
     */
    static uint32_t CalculateChecksum(const uint8_t* data, size_t length);
    
    /**
     * @brief Write uint16_t to buffer (little-endian)
     */
    static void WriteUInt16(uint8_t* buffer, uint16_t value);
    
    /**
     * @brief Write uint32_t to buffer (little-endian)
     */
    static void WriteUInt32(uint8_t* buffer, uint32_t value);
    
    /**
     * @brief Read uint16_t from buffer (little-endian)
     */
    static uint16_t ReadUInt16(const uint8_t* buffer);
    
    /**
     * @brief Read uint32_t from buffer (little-endian)
     */
    static uint32_t ReadUInt32(const uint8_t* buffer);
};

} // namespace P2P

