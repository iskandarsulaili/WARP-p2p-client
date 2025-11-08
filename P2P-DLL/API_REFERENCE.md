# P2P Network DLL - API Reference

**Version:** 1.0.0  
**Last Updated:** November 8, 2025  
**Platform:** Windows 10/11 (x64)

---

## Table of Contents

1. [Exported DLL Functions](#exported-dll-functions)
2. [Core Classes](#core-classes)
3. [Configuration](#configuration)
4. [Data Types](#data-types)
5. [Callback Signatures](#callback-signatures)

---

## Exported DLL Functions

These are the C-style functions exported by `p2p_network.dll` for use by the Ragnarok Online client.

### P2P_Initialize

```c
extern "C" __declspec(dllexport) bool P2P_Initialize(const char* config_path);
```

**Description:** Initializes the P2P networking system with optional configuration file.

**Parameters:**

- `config_path` - Path to JSON configuration file (can be `NULL` to use default)

**Returns:** `true` if initialized successfully, `false` otherwise

**Example:**

```cpp
// From RO client or NEMO patch
if (P2P_Initialize("config/p2p_config.json")) {
    printf("P2P initialized successfully\n");
} else {
    printf("P2P initialization failed: %s\n", P2P_GetLastError());
}
```

**Notes:**

- Must be called before `P2P_Start()`
- Can be called multiple times (subsequent calls are ignored)
- If `config_path` is `NULL`, uses default configuration from DLL directory

---

### P2P_Start

```c
extern "C" __declspec(dllexport) bool P2P_Start(const char* player_id, const char* user_id);
```

**Description:** Starts P2P networking for the specified player.

**Parameters:**

- `player_id` - Character ID (used as peer identifier)
- `user_id` - Account ID (for authentication)

**Returns:** `true` if started successfully, `false` otherwise

**Example:**

```cpp
// Start P2P for character ID 150001, account ID 2000001
if (P2P_Start("150001", "2000001")) {
    printf("P2P networking started\n");
}
```

**Notes:**

- Requires `P2P_Initialize()` to be called first
- Connects to coordinator server and begins WebRTC signaling
- Returns `true` if already started (not an error)

---

### P2P_Shutdown

```c
extern "C" __declspec(dllexport) void P2P_Shutdown();
```

**Description:** Stops P2P networking and releases all resources.

**Parameters:** None

**Returns:** None (void)

**Example:**

```cpp
// Called when client exits or player logs out
P2P_Shutdown();
```

**Notes:**

- Safe to call multiple times
- Automatically called when DLL is unloaded
- Closes all peer connections gracefully

---

### P2P_IsEnabled

```c
extern "C" __declspec(dllexport) bool P2P_IsEnabled();
```

**Description:** Checks if P2P networking is enabled in configuration.

**Parameters:** None

**Returns:** `true` if P2P is enabled, `false` otherwise

**Example:**

```cpp
if (P2P_IsEnabled()) {
    // P2P is configured to be enabled
    P2P_Start(player_id, user_id);
}
```

---

### P2P_IsActive

```c
extern "C" __declspec(dllexport) bool P2P_IsActive();
```

**Description:** Checks if P2P networking is currently active and running.

**Parameters:** None

**Returns:** `true` if P2P is active, `false` otherwise

**Example:**

```cpp
if (P2P_IsActive()) {
    // P2P is running, can send packets
    SendP2PPacket(data, size);
}
```

---

### P2P_GetStatus

```c
extern "C" __declspec(dllexport) const char* P2P_GetStatus();
```

**Description:** Returns current P2P status as a JSON string.

**Parameters:** None

**Returns:** JSON string with status information (pointer valid until next call)

**Example:**

```cpp
const char* status_json = P2P_GetStatus();
printf("Status: %s\n", status_json);
```

**JSON Format:**

```json
{
  "dll_initialized": true,
  "p2p_active": true,
  "p2p_enabled": true,
  "network_active": true,
  "last_error": "",
  "coordinator_url": "https://coordinator.example.com/api/v1",
  "max_peers": 50,
  "encryption_enabled": true
}
```

**Notes:**

- Returned pointer is valid until next call to `P2P_GetStatus()`
- Do not free the returned pointer
- Parse JSON to extract individual fields

---

### P2P_GetLastError

```c
extern "C" __declspec(dllexport) const char* P2P_GetLastError();
```

**Description:** Returns the last error message.

**Parameters:** None

**Returns:** Error message string (empty if no error)

**Example:**

```cpp
if (!P2P_Start(player_id, user_id)) {
    printf("Error: %s\n", P2P_GetLastError());
}
```

---

## Core Classes

These classes are used internally by the DLL. They are documented here for developers who want to understand the implementation or extend the DLL.

### NetworkManager

**Header:** `include/NetworkManager.h`
**Namespace:** `P2P`

**Description:** Central orchestrator for all P2P networking operations.

**Public Methods:**

```cpp
static NetworkManager& GetInstance();
```

- Get singleton instance

```cpp
bool Initialize(const std::string& peer_id);
```

- Initialize network manager with peer ID
- Returns `true` on success

```cpp
void Shutdown();
```

- Shutdown network manager and release resources

```cpp
bool Start();
```

- Start P2P networking
- Returns `true` on success

```cpp
void Stop();
```

- Stop P2P networking

```cpp
bool IsActive() const;
```

- Check if P2P is currently active
- Returns `true` if active

```cpp
void OnZoneChange(const std::string& zone_id);
```

- Handle zone change event
- Disconnects from old zone peers, connects to new zone peers

```cpp
bool SendPacket(const Packet& packet);
```

- Send packet via P2P or server (routing decision made automatically)
- Returns `true` if sent successfully

---

### ConfigManager

**Header:** `include/ConfigManager.h`
**Namespace:** `P2P`

**Description:** Manages configuration loading and validation.

**Public Methods:**

```cpp
static ConfigManager& GetInstance();
```

- Get singleton instance

```cpp
bool LoadFromFile(const std::string& config_path);
```

- Load configuration from JSON file
- Returns `true` on success

```cpp
bool LoadFromString(const std::string& json_str);
```

- Load configuration from JSON string
- Returns `true` on success

```cpp
bool Validate() const;
```

- Validate current configuration
- Returns `true` if valid

```cpp
const Config& GetConfig() const;
```

- Get complete configuration structure

```cpp
const CoordinatorConfig& GetCoordinatorConfig() const;
const WebRTCConfig& GetWebRTCConfig() const;
const P2PConfig& GetP2PConfig() const;
const SecurityConfig& GetSecurityConfig() const;
const LoggingConfig& GetLoggingConfig() const;
const ZonesConfig& GetZonesConfig() const;
const PerformanceConfig& GetPerformanceConfig() const;
const HostConfig& GetHostConfig() const;
```

- Get specific configuration sections

```cpp
bool IsP2PEnabled() const;
```

- Check if P2P is enabled globally

```cpp
bool IsZoneP2PEnabled(const std::string& zone_id) const;
```

- Check if specific zone has P2P enabled

```cpp
void UpdateJWTToken(const std::string& token);
```

- Update JWT authentication token

---

### WebRTCManager

**Header:** `include/WebRTCManager.h`
**Namespace:** `P2P`

**Description:** Manages multiple WebRTC peer connections.

**Public Methods:**

```cpp
std::shared_ptr<WebRTCPeerConnection> CreatePeerConnection(const std::string& peer_id);
```

- Create new peer connection
- Returns shared pointer to peer connection

```cpp
std::shared_ptr<WebRTCPeerConnection> GetPeerConnection(const std::string& peer_id);
```

- Get existing peer connection by ID
- Returns `nullptr` if not found

```cpp
void RemovePeerConnection(const std::string& peer_id);
```

- Remove and close peer connection

```cpp
std::vector<std::string> GetAllPeerIds() const;
```

- Get list of all active peer IDs

```cpp
bool SendDataToPeer(const std::string& peer_id, const uint8_t* data, size_t size);
```

- Send data to specific peer
- Returns `true` on success

```cpp
void BroadcastData(const uint8_t* data, size_t size);
```

- Broadcast data to all connected peers

---

### WebRTCPeerConnection

**Header:** `include/WebRTCPeerConnection.h`
**Namespace:** `P2P`

**Description:** Represents a single WebRTC peer-to-peer connection.

**See:** [WEBRTC_GUIDE.md](WEBRTC_GUIDE.md) for detailed WebRTC API documentation

**Key Methods:**

```cpp
bool Initialize(const std::vector<std::string>& stun_servers,
                const std::vector<std::string>& turn_servers);
```

- Initialize peer connection with STUN/TURN servers

```cpp
bool CreateOffer(std::string& sdp_out);
bool CreateAnswer(std::string& sdp_out);
bool SetRemoteDescription(const std::string& sdp);
bool AddIceCandidate(const std::string& candidate);
```

- WebRTC signaling methods

```cpp
bool SendData(const uint8_t* data, size_t size);
```

- Send data via data channel

```cpp
bool IsConnected() const;
```

- Check if peer is connected

```cpp
std::string GetPeerId() const;
```

- Get peer identifier

**Callbacks:**

```cpp
using OnDataCallback = std::function<void(const uint8_t*, size_t)>;
using OnStateChangeCallback = std::function<void(bool connected)>;
using OnIceCandidateCallback = std::function<void(const std::string& candidate)>;

void SetOnDataCallback(OnDataCallback callback);
void SetOnStateChangeCallback(OnStateChangeCallback callback);
void SetOnIceCandidateCallback(OnIceCandidateCallback callback);
```

---

### SignalingClient

**Header:** `include/SignalingClient.h`
**Namespace:** `P2P`

**Description:** WebSocket client for signaling server communication (SDP/ICE exchange).

**Public Methods:**

```cpp
bool Connect(const std::string& url);
```

- Connect to signaling server WebSocket
- Returns `true` on success

```cpp
void Disconnect();
```

- Disconnect from signaling server

```cpp
bool IsConnected() const;
```

- Check if connected to signaling server

```cpp
bool SendOffer(const std::string& peer_id, const std::string& sdp);
bool SendAnswer(const std::string& peer_id, const std::string& sdp);
bool SendIceCandidate(const std::string& peer_id, const std::string& candidate);
```

- Send signaling messages to remote peer

**Callbacks:**

```cpp
using OnOfferCallback = std::function<void(const std::string& peer_id, const std::string& sdp)>;
using OnAnswerCallback = std::function<void(const std::string& peer_id, const std::string& sdp)>;
using OnIceCandidateCallback = std::function<void(const std::string& peer_id, const std::string& candidate)>;

void SetOnOfferCallback(OnOfferCallback callback);
void SetOnAnswerCallback(OnAnswerCallback callback);
void SetOnIceCandidateCallback(OnIceCandidateCallback callback);
```

---

### PacketRouter

**Header:** `include/PacketRouter.h`
**Namespace:** `P2P`

**Description:** Routes packets between P2P and traditional server based on packet type.

**Public Methods:**

```cpp
RouteDecision DetermineRoute(const Packet& packet);
```

- Determine routing decision for packet
- Returns `RouteDecision` enum value

```cpp
bool ShouldUseP2P(uint16_t packet_id) const;
```

- Check if packet type should use P2P
- Returns `true` if P2P should be used

---

### SecurityManager

**Header:** `include/SecurityManager.h`
**Namespace:** `P2P`

**Description:** Handles encryption/decryption of P2P packets.

**Public Methods:**

```cpp
bool Encrypt(const uint8_t* plaintext, size_t plaintext_len,
             uint8_t* ciphertext, size_t& ciphertext_len);
```

- Encrypt data using AES-256-GCM
- Returns `true` on success

```cpp
bool Decrypt(const uint8_t* ciphertext, size_t ciphertext_len,
             uint8_t* plaintext, size_t& plaintext_len);
```

- Decrypt data using AES-256-GCM
- Returns `true` on success

```cpp
bool GenerateKey();
```

- Generate new encryption key
- Returns `true` on success

---

## Configuration

### Configuration File Format

The P2P DLL uses a JSON configuration file (`p2p_config.json`).

**Location:** Same directory as `p2p_network.dll` or specified path in `P2P_Initialize()`

**Complete Example:**

```json
{
  "coordinator": {
    "rest_api_url": "https://coordinator.example.com/api/v1",
    "websocket_url": "wss://coordinator.example.com/api/v1/signaling/ws",
    "timeout_ms": 5000,
    "reconnect_max_attempts": 5,
    "reconnect_backoff_ms": 1000
  },
  "webrtc": {
    "stun_servers": [
      "stun:stun.l.google.com:19302",
      "stun:stun1.l.google.com:19302"
    ],
    "turn_servers": ["turn:username:password@turn.example.com:3478"],
    "ice_transport_policy": "all",
    "bundle_policy": "balanced",
    "rtcp_mux_policy": "require",
    "enable_dtls": true,
    "enable_rtp_data_channels": false
  },
  "p2p": {
    "enabled": true,
    "max_peers": 50,
    "max_packet_size_bytes": 65536,
    "max_bandwidth_mbps": 10,
    "target_bitrate_kbps": 1000,
    "enable_congestion_control": true,
    "packet_queue_size": 1000
  },
  "security": {
    "enable_encryption": true,
    "enable_authentication": true,
    "api_key": "your-api-key-here",
    "jwt_token": "",
    "certificate_validation": true,
    "tls_version": "1.3"
  },
  "logging": {
    "level": "INFO",
    "file": "logs/p2p_network.log",
    "max_file_size_mb": 10,
    "max_files": 5,
    "console_output": false,
    "async_logging": true
  },
  "zones": {
    "p2p_enabled_zones": ["prontera", "geffen", "payon", "alberta"],
    "fallback_on_failure": true,
    "zone_transition_timeout_ms": 3000
  },
  "performance": {
    "worker_threads": 4,
    "io_thread_pool_size": 2,
    "enable_packet_batching": true,
    "packet_batch_size": 10,
    "packet_batch_timeout_ms": 5
  },
  "host": {
    "enable_hosting": false,
    "max_players": 100,
    "max_zones": 10,
    "heartbeat_interval_seconds": 30,
    "quality_report_interval_seconds": 60
  }
}
```

### Configuration Options Reference

| Section         | Option                            | Type   | Description                                   | Default                |
| --------------- | --------------------------------- | ------ | --------------------------------------------- | ---------------------- |
| **coordinator** | `rest_api_url`                    | string | Coordinator REST API endpoint                 | Required               |
|                 | `websocket_url`                   | string | Signaling WebSocket endpoint                  | Required               |
|                 | `timeout_ms`                      | int    | HTTP request timeout (ms)                     | 5000                   |
|                 | `reconnect_max_attempts`          | int    | Max reconnection attempts                     | 5                      |
|                 | `reconnect_backoff_ms`            | int    | Reconnection backoff (ms)                     | 1000                   |
| **webrtc**      | `stun_servers`                    | array  | STUN server URLs                              | Google STUN            |
|                 | `turn_servers`                    | array  | TURN server URLs                              | []                     |
|                 | `ice_transport_policy`            | string | ICE transport policy                          | "all"                  |
|                 | `bundle_policy`                   | string | Bundle policy                                 | "balanced"             |
|                 | `rtcp_mux_policy`                 | string | RTCP mux policy                               | "require"              |
|                 | `enable_dtls`                     | bool   | Enable DTLS encryption                        | true                   |
|                 | `enable_rtp_data_channels`        | bool   | Enable RTP data channels                      | false                  |
| **p2p**         | `enabled`                         | bool   | Enable P2P networking                         | true                   |
|                 | `max_peers`                       | int    | Maximum concurrent peers                      | 50                     |
|                 | `max_packet_size_bytes`           | int    | Maximum packet size                           | 65536                  |
|                 | `max_bandwidth_mbps`              | int    | Maximum bandwidth (Mbps)                      | 10                     |
|                 | `target_bitrate_kbps`             | int    | Target bitrate (Kbps)                         | 1000                   |
|                 | `enable_congestion_control`       | bool   | Enable congestion control                     | true                   |
|                 | `packet_queue_size`               | int    | Packet queue size                             | 1000                   |
| **security**    | `enable_encryption`               | bool   | Enable packet encryption                      | true                   |
|                 | `enable_authentication`           | bool   | Enable authentication                         | true                   |
|                 | `api_key`                         | string | API key for coordinator                       | Required               |
|                 | `jwt_token`                       | string | JWT authentication token                      | ""                     |
|                 | `certificate_validation`          | bool   | Validate SSL certificates                     | true                   |
|                 | `tls_version`                     | string | TLS version                                   | "1.3"                  |
| **logging**     | `level`                           | string | Log level (TRACE/DEBUG/INFO/WARN/ERROR/FATAL) | "INFO"                 |
|                 | `file`                            | string | Log file path                                 | "logs/p2p_network.log" |
|                 | `max_file_size_mb`                | int    | Max log file size (MB)                        | 10                     |
|                 | `max_files`                       | int    | Max log files to keep                         | 5                      |
|                 | `console_output`                  | bool   | Output to console                             | false                  |
|                 | `async_logging`                   | bool   | Use async logging                             | true                   |
| **zones**       | `p2p_enabled_zones`               | array  | Zones with P2P enabled                        | []                     |
|                 | `fallback_on_failure`             | bool   | Fallback to server on P2P failure             | true                   |
|                 | `zone_transition_timeout_ms`      | int    | Zone transition timeout (ms)                  | 3000                   |
| **performance** | `worker_threads`                  | int    | Number of worker threads                      | 4                      |
|                 | `io_thread_pool_size`             | int    | I/O thread pool size                          | 2                      |
|                 | `enable_packet_batching`          | bool   | Enable packet batching                        | true                   |
|                 | `packet_batch_size`               | int    | Packet batch size                             | 10                     |
|                 | `packet_batch_timeout_ms`         | int    | Packet batch timeout (ms)                     | 5                      |
| **host**        | `enable_hosting`                  | bool   | Enable zone hosting                           | false                  |
|                 | `max_players`                     | int    | Max players per zone                          | 100                    |
|                 | `max_zones`                       | int    | Max zones to host                             | 10                     |
|                 | `heartbeat_interval_seconds`      | int    | Heartbeat interval (s)                        | 30                     |
|                 | `quality_report_interval_seconds` | int    | Quality report interval (s)                   | 60                     |

---

## Data Types

### Enumerations

#### ConnectionState

```cpp
enum class ConnectionState {
    NEW,           // Connection created but not started
    CONNECTING,    // Connection in progress
    CONNECTED,     // Connection established
    DISCONNECTED,  // Connection lost
    FAILED,        // Connection failed
    CLOSED         // Connection closed
};
```

#### RouteDecision

```cpp
enum class RouteDecision {
    P2P,        // Send via P2P
    SERVER,     // Send to centralized server
    BROADCAST,  // Broadcast to all peers
    DROP        // Drop packet
};
```

#### LogLevel

```cpp
enum class LogLevel {
    TRACE,   // Verbose debugging
    DEBUG,   // Debug information
    INFO,    // Informational messages
    WARN,    // Warning messages
    ERR,     // Error messages
    FATAL    // Fatal errors
};
```

### Structures

#### Packet

```cpp
struct Packet {
    uint16_t packet_id;           // Packet identifier
    uint16_t type;                // Packet type for routing
    std::vector<uint8_t> data;    // Packet data
    size_t length;                // Data length
};
```

**Usage:**

```cpp
Packet packet;
packet.packet_id = 0x0088;  // Movement packet
packet.type = 0x0088;
packet.data = {0x88, 0x00, 0x64, 0x00, 0xC8, 0x00, 0x03};
packet.length = packet.data.size();

NetworkManager::GetInstance().SendPacket(packet);
```

#### PeerInfo

```cpp
struct PeerInfo {
    std::string peer_id;           // Peer identifier
    std::string player_id;         // Player identifier
    ConnectionState state;         // Connection state
    float latency_ms;              // Latency in milliseconds
    float packet_loss_percent;     // Packet loss percentage
};
```

#### SessionInfo

```cpp
struct SessionInfo {
    std::string session_id;        // Session identifier
    std::string zone_id;           // Zone identifier
    std::string host_id;           // Host peer identifier
    std::vector<std::string> peer_ids;  // List of peer IDs in session
    int max_players;               // Maximum players
    int current_players;           // Current player count
};
```

---

## Callback Signatures

### WebRTC Callbacks

```cpp
// Called when data is received from peer
using OnDataCallback = std::function<void(const uint8_t* data, size_t size)>;

// Called when connection state changes
using OnStateChangeCallback = std::function<void(bool connected)>;

// Called when ICE candidate is discovered
using OnIceCandidateCallback = std::function<void(const std::string& candidate)>;
```

**Example:**

```cpp
peer->SetOnDataCallback([](const uint8_t* data, size_t size) {
    // Process received data
    ProcessPacket(data, size);
});

peer->SetOnStateChangeCallback([](bool connected) {
    if (connected) {
        LOG_INFO("Peer connected!");
    } else {
        LOG_WARN("Peer disconnected!");
    }
});
```

### Signaling Callbacks

```cpp
// Called when SDP offer is received
using OnOfferCallback = std::function<void(const std::string& peer_id, const std::string& sdp)>;

// Called when SDP answer is received
using OnAnswerCallback = std::function<void(const std::string& peer_id, const std::string& sdp)>;

// Called when ICE candidate is received
using OnIceCandidateCallback = std::function<void(const std::string& peer_id, const std::string& candidate)>;
```

**Example:**

```cpp
signaling->SetOnOfferCallback([&](const std::string& peer_id, const std::string& sdp) {
    // Create peer connection and send answer
    auto peer = webrtc_manager.CreatePeerConnection(peer_id);
    peer->SetRemoteDescription(sdp);

    std::string answer;
    peer->CreateAnswer(answer);
    signaling->SendAnswer(peer_id, answer);
});
```

---

## Usage Examples

### Example 1: Initialize and Start P2P

```cpp
// From RO client or NEMO patch
#include <windows.h>

typedef bool (*P2P_InitializeFunc)(const char*);
typedef bool (*P2P_StartFunc)(const char*, const char*);
typedef const char* (*P2P_GetStatusFunc)();

int main() {
    // Load DLL
    HMODULE dll = LoadLibrary("p2p_network.dll");
    if (!dll) {
        printf("Failed to load DLL\n");
        return 1;
    }

    // Get function pointers
    auto P2P_Initialize = (P2P_InitializeFunc)GetProcAddress(dll, "P2P_Initialize");
    auto P2P_Start = (P2P_StartFunc)GetProcAddress(dll, "P2P_Start");
    auto P2P_GetStatus = (P2P_GetStatusFunc)GetProcAddress(dll, "P2P_GetStatus");

    // Initialize
    if (!P2P_Initialize("config/p2p_config.json")) {
        printf("Initialization failed\n");
        return 1;
    }

    // Start P2P
    if (!P2P_Start("150001", "2000001")) {
        printf("Start failed\n");
        return 1;
    }

    // Check status
    const char* status = P2P_GetStatus();
    printf("Status: %s\n", status);

    return 0;
}
```

### Example 2: Check P2P Status

```cpp
#include <nlohmann/json.hpp>

void CheckP2PStatus() {
    const char* status_json = P2P_GetStatus();

    try {
        auto status = nlohmann::json::parse(status_json);

        if (status["p2p_active"]) {
            printf("P2P is active\n");
            printf("Connected peers: %d\n", status["connected_peers"].get<int>());
            printf("Coordinator: %s\n", status["coordinator_url"].get<std::string>().c_str());
        } else {
            printf("P2P is not active\n");
            if (!status["last_error"].get<std::string>().empty()) {
                printf("Error: %s\n", status["last_error"].get<std::string>().c_str());
            }
        }
    } catch (const std::exception& e) {
        printf("Failed to parse status: %s\n", e.what());
    }
}
```

---

## Related Documentation

- **[BUILD_GUIDE.md](BUILD_GUIDE.md)** - Build instructions and troubleshooting
- **[WEBRTC_GUIDE.md](WEBRTC_GUIDE.md)** - WebRTC implementation details
- **[DEPLOYMENT_GUIDE.md](DEPLOYMENT_GUIDE.md)** - Deployment and integration guide
- **[README.md](README.md)** - Project overview

---

**API Reference Version:** 1.0.0
**Last Updated:** November 8, 2025
**Maintained by:** rAthena AI World Development Team
