# WebRTC Implementation Guide

**Library:** libdatachannel 0.23.2  
**Language:** C++17  
**Platform:** Windows 10/11 (x64)  
**Last Updated:** November 8, 2025

---

## Table of Contents

1. [Introduction](#introduction)
2. [Why libdatachannel?](#why-libdatachannel)
3. [Architecture Overview](#architecture-overview)
4. [WebRTCPeerConnection API](#webrtcpeerconnection-api)
5. [WebRTCManager API](#webrtcmanager-api)
6. [Code Examples](#code-examples)
7. [Signaling Flow](#signaling-flow)
8. [STUN/TURN Configuration](#stunturn-configuration)
9. [Testing](#testing)
10. [Troubleshooting](#troubleshooting)
11. [Performance Tuning](#performance-tuning)

---

## Introduction

This guide explains the WebRTC implementation in the P2P Network DLL. WebRTC (Web Real-Time Communication) enables peer-to-peer data channels between Ragnarok Online clients, allowing direct communication without routing through the game server.

### What is WebRTC?

WebRTC is an open-source project that provides:

- **Peer-to-peer connections** - Direct communication between clients
- **NAT traversal** - Works behind firewalls and routers using STUN/TURN
- **Data channels** - Reliable or unreliable data transmission
- **Built-in security** - DTLS encryption for all data

### Use Cases in RO

- **Player movement packets** - Broadcast position updates to nearby players
- **Chat messages** - Send chat directly to players in the same zone
- **Emotes and actions** - Share emotes without server relay
- **Zone synchronization** - Keep zone state synchronized across peers

---

## Why libdatachannel?

We chose **libdatachannel** over Google's libwebrtc for several reasons:

| Feature          | libdatachannel     | Google libwebrtc       |
| ---------------- | ------------------ | ---------------------- |
| **Size**         | ~500 KB            | 100+ MB                |
| **API**          | Modern C++17       | C-style API            |
| **Focus**        | Data channels only | Full WebRTC stack      |
| **Integration**  | Easy (vcpkg)       | Complex (custom build) |
| **Dependencies** | Minimal            | Many                   |
| **Build Time**   | ~10 minutes        | ~2 hours               |

**libdatachannel** is perfect for our use case because:

- ✅ We only need data channels (no audio/video)
- ✅ Lightweight and fast
- ✅ Easy to integrate with vcpkg
- ✅ Modern C++ API with std::function callbacks
- ✅ Active development and good documentation

---

## Architecture Overview

### Component Hierarchy

```
NetworkManager
    │
    ├── WebRTCManager
    │       │
    │       ├── WebRTCPeerConnection (Peer 1)
    │       │       └── rtc::DataChannel
    │       │
    │       ├── WebRTCPeerConnection (Peer 2)
    │       │       └── rtc::DataChannel
    │       │
    │       └── WebRTCPeerConnection (Peer N)
    │               └── rtc::DataChannel
    │
    └── SignalingClient (WebSocket)
            └── Coordinator Server
```

### Data Flow

```
┌─────────────────────────────────────────────────────────────┐
│                     Local Client                             │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  Application Layer                                      │ │
│  │  (RO Client calls P2P_SendData)                        │ │
│  └────────────────────────────────────────────────────────┘ │
│                          │                                   │
│                          ▼                                   │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  WebRTCManager                                          │ │
│  │  (Routes to appropriate peer)                          │ │
│  └────────────────────────────────────────────────────────┘ │
│                          │                                   │
│                          ▼                                   │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  WebRTCPeerConnection                                   │ │
│  │  (Encrypts and sends via data channel)                 │ │
│  └────────────────────────────────────────────────────────┘ │
│                          │                                   │
│                          ▼                                   │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  rtc::DataChannel (libdatachannel)                     │ │
│  │  (DTLS encryption, SCTP transport)                     │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
                          │
                          │ P2P Connection
                          │ (Direct UDP)
                          ▼
┌─────────────────────────────────────────────────────────────┐
│                     Remote Peer                              │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  rtc::DataChannel                                       │ │
│  │  (Receives encrypted data)                             │ │
│  └────────────────────────────────────────────────────────┘ │
│                          │                                   │
│                          ▼                                   │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  WebRTCPeerConnection                                   │ │
│  │  (Decrypts and processes)                              │ │
│  └────────────────────────────────────────────────────────┘ │
│                          │                                   │
│                          ▼                                   │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  Application Layer                                      │ │
│  │  (OnDataCallback triggered)                            │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

---

## WebRTCPeerConnection API

### Class Overview

`WebRTCPeerConnection` represents a single peer-to-peer connection to another client.

**Header:** `include/WebRTCPeerConnection.h`  
**Implementation:** `src/webrtc/WebRTCPeerConnection.cpp`

### Public Methods

#### Initialize

```cpp
bool Initialize(const std::vector<std::string>& stun_servers,
                const std::vector<std::string>& turn_servers);
```

**Description:** Initializes the peer connection with STUN/TURN servers.

**Parameters:**

- `stun_servers` - List of STUN server URLs (e.g., "stun:stun.l.google.com:19302")
- `turn_servers` - List of TURN server URLs (e.g., "turn:turn.example.com:3478")

**Returns:** `true` on success, `false` on failure

**Example:**

```cpp
WebRTCPeerConnection peer("player123");
std::vector<std::string> stun = {"stun:stun.l.google.com:19302"};
std::vector<std::string> turn = {"turn:turn.example.com:3478"};

if (!peer.Initialize(stun, turn)) {
    LOG_ERROR("Failed to initialize peer connection");
}
```

#### CreateOffer

```cpp
bool CreateOffer(std::string& sdp_out);
```

**Description:** Creates an SDP offer to initiate a connection (caller side).

**Parameters:**

- `sdp_out` - Output parameter that receives the SDP offer string

**Returns:** `true` on success, `false` on failure

**Example:**

```cpp
std::string offer_sdp;
if (peer.CreateOffer(offer_sdp)) {
    // Send offer_sdp to remote peer via signaling server
    signaling_client->SendOffer("player123", offer_sdp);
}
```

#### CreateAnswer

```cpp
bool CreateAnswer(std::string& sdp_out);
```

**Description:** Creates an SDP answer in response to an offer (callee side).

**Parameters:**

- `sdp_out` - Output parameter that receives the SDP answer string

**Returns:** `true` on success, `false` on failure

**Example:**

```cpp
std::string answer_sdp;
if (peer.CreateAnswer(answer_sdp)) {
    // Send answer_sdp back to caller via signaling server
    signaling_client->SendAnswer("player123", answer_sdp);
}
```

#### SetRemoteDescription

```cpp
bool SetRemoteDescription(const std::string& sdp);
```

**Description:** Sets the remote peer's SDP (offer or answer).

**Parameters:**

- `sdp` - SDP string received from remote peer

**Returns:** `true` on success, `false` on failure

**Example:**

```cpp
// Received offer from remote peer
std::string remote_offer = /* from signaling */;
if (peer.SetRemoteDescription(remote_offer)) {
    // Now create and send answer
    std::string answer;
    peer.CreateAnswer(answer);
    signaling_client->SendAnswer("player123", answer);
}
```

#### AddIceCandidate

```cpp
bool AddIceCandidate(const std::string& candidate);
```

**Description:** Adds an ICE candidate received from the remote peer.

**Parameters:**

- `candidate` - ICE candidate string

**Returns:** `true` on success, `false` on failure

**Example:**

```cpp
// Received ICE candidate from remote peer
std::string ice_candidate = /* from signaling */;
peer.AddIceCandidate(ice_candidate);
```

#### SendData

```cpp
bool SendData(const uint8_t* data, size_t size);
```

**Description:** Sends binary data to the remote peer via the data channel.

**Parameters:**

- `data` - Pointer to binary data buffer
- `size` - Size of data in bytes

**Returns:** `true` on success, `false` on failure

**Example:**

```cpp
// Send player movement packet
struct MovementPacket {
    uint16_t x;
    uint16_t y;
    uint8_t direction;
};

MovementPacket packet = {100, 200, 3};
if (!peer.SendData(reinterpret_cast<uint8_t*>(&packet), sizeof(packet))) {
    LOG_ERROR("Failed to send movement packet");
}
```

#### IsConnected

```cpp
bool IsConnected() const;
```

**Description:** Checks if the peer connection is established and data channel is open.

**Returns:** `true` if connected, `false` otherwise

**Example:**

```cpp
if (peer.IsConnected()) {
    // Safe to send data
    peer.SendData(data, size);
} else {
    LOG_WARN("Peer not connected, queuing packet");
}
```

#### GetPeerId

```cpp
std::string GetPeerId() const;
```

**Description:** Gets the peer ID (player ID) for this connection.

**Returns:** Peer ID string

### Callback Functions

#### OnDataCallback

```cpp
using OnDataCallback = std::function<void(const uint8_t*, size_t)>;
void SetOnDataCallback(OnDataCallback callback);
```

**Description:** Called when data is received from the remote peer.

**Parameters:**

- `data` - Pointer to received data buffer
- `size` - Size of received data in bytes

**Example:**

```cpp
peer.SetOnDataCallback([](const uint8_t* data, size_t size) {
    LOG_INFO("Received " + std::to_string(size) + " bytes from peer");

    // Parse and process packet
    if (size >= sizeof(PacketHeader)) {
        PacketHeader* header = (PacketHeader*)data;
        switch (header->type) {
            case PACKET_MOVEMENT:
                HandleMovementPacket(data, size);
                break;
            case PACKET_CHAT:
                HandleChatPacket(data, size);
                break;
        }
    }
});
```

#### OnStateChangeCallback

```cpp
using OnStateChangeCallback = std::function<void(bool connected)>;
void SetOnStateChangeCallback(OnStateChangeCallback callback);
```

**Description:** Called when the connection state changes.

**Parameters:**

- `connected` - `true` if now connected, `false` if disconnected

**Example:**

```cpp
peer.SetOnStateChangeCallback([](bool connected) {
    if (connected) {
        LOG_INFO("Peer connection established!");
    } else {
        LOG_WARN("Peer connection lost!");
    }
});
```

#### OnIceCandidateCallback

```cpp
using OnIceCandidateCallback = std::function<void(const std::string& candidate)>;
void SetOnIceCandidateCallback(OnIceCandidateCallback callback);
```

**Description:** Called when a local ICE candidate is discovered.

**Parameters:**

- `candidate` - ICE candidate string to send to remote peer

**Example:**

```cpp
peer.SetOnIceCandidateCallback([&](const std::string& candidate) {
    // Send ICE candidate to remote peer via signaling
    signaling_client->SendIceCandidate("player123", candidate);
});
```

---

## WebRTCManager API

### Class Overview

`WebRTCManager` manages multiple peer connections.

**Header:** `include/WebRTCManager.h`
**Implementation:** `src/webrtc/WebRTCManager.cpp`

### Public Methods

#### CreatePeerConnection

```cpp
std::shared_ptr<WebRTCPeerConnection> CreatePeerConnection(const std::string& peer_id);
```

**Description:** Creates a new peer connection for the specified peer ID.

**Parameters:**

- `peer_id` - Unique identifier for the peer (e.g., player ID)

**Returns:** Shared pointer to the new peer connection

**Example:**

```cpp
WebRTCManager manager;
auto peer = manager.CreatePeerConnection("player123");

// Set up callbacks
peer->SetOnDataCallback([](const uint8_t* data, size_t size) {
    // Handle received data
});

// Initialize and create offer
std::vector<std::string> stun = {"stun:stun.l.google.com:19302"};
std::vector<std::string> turn = {};
peer->Initialize(stun, turn);

std::string offer;
peer->CreateOffer(offer);
```

#### GetPeerConnection

```cpp
std::shared_ptr<WebRTCPeerConnection> GetPeerConnection(const std::string& peer_id);
```

**Description:** Retrieves an existing peer connection by peer ID.

**Parameters:**

- `peer_id` - Peer ID to look up

**Returns:** Shared pointer to peer connection, or `nullptr` if not found

**Example:**

```cpp
auto peer = manager.GetPeerConnection("player123");
if (peer && peer->IsConnected()) {
    peer->SendData(data, size);
}
```

#### RemovePeerConnection

```cpp
void RemovePeerConnection(const std::string& peer_id);
```

**Description:** Removes and closes a peer connection.

**Parameters:**

- `peer_id` - Peer ID to remove

**Example:**

```cpp
// Player left the zone
manager.RemovePeerConnection("player123");
```

#### GetAllPeerIds

```cpp
std::vector<std::string> GetAllPeerIds() const;
```

**Description:** Gets a list of all active peer IDs.

**Returns:** Vector of peer ID strings

**Example:**

```cpp
auto peers = manager.GetAllPeerIds();
LOG_INFO("Active peers: " + std::to_string(peers.size()));

for (const auto& peer_id : peers) {
    auto peer = manager.GetPeerConnection(peer_id);
    if (peer && peer->IsConnected()) {
        // Broadcast message to all connected peers
        peer->SendData(broadcast_data, broadcast_size);
    }
}
```

---

## Code Examples

### Example 1: Complete Connection Flow (Caller Side)

```cpp
#include "WebRTCManager.h"
#include "SignalingClient.h"

// Initialize manager
WebRTCManager webrtc_manager;
SignalingClient signaling;

// Connect to signaling server
signaling.Connect("wss://coordinator.example.com/signaling");

// Create peer connection
auto peer = webrtc_manager.CreatePeerConnection("player456");

// Set up callbacks
peer->SetOnDataCallback([](const uint8_t* data, size_t size) {
    LOG_INFO("Received data: " + std::to_string(size) + " bytes");
    // Process received packet
});

peer->SetOnStateChangeCallback([](bool connected) {
    if (connected) {
        LOG_INFO("✅ Connected to peer!");
    } else {
        LOG_WARN("❌ Disconnected from peer");
    }
});

peer->SetOnIceCandidateCallback([&](const std::string& candidate) {
    // Send ICE candidate via signaling
    signaling.SendIceCandidate("player456", candidate);
});

// Initialize with STUN servers
std::vector<std::string> stun = {
    "stun:stun.l.google.com:19302",
    "stun:stun1.l.google.com:19302"
};
std::vector<std::string> turn = {};

if (!peer->Initialize(stun, turn)) {
    LOG_ERROR("Failed to initialize peer connection");
    return;
}

// Create and send offer
std::string offer_sdp;
if (peer->CreateOffer(offer_sdp)) {
    signaling.SendOffer("player456", offer_sdp);
    LOG_INFO("Sent offer to player456");
}

// Wait for answer from signaling server
signaling.SetOnAnswerCallback([&](const std::string& peer_id, const std::string& answer_sdp) {
    if (peer_id == "player456") {
        peer->SetRemoteDescription(answer_sdp);
        LOG_INFO("Received answer from player456");
    }
});

// Handle incoming ICE candidates
signaling.SetOnIceCandidateCallback([&](const std::string& peer_id, const std::string& candidate) {
    if (peer_id == "player456") {
        peer->AddIceCandidate(candidate);
    }
});
```

### Example 2: Answering a Connection (Callee Side)

```cpp
// Received offer from signaling server
signaling.SetOnOfferCallback([&](const std::string& peer_id, const std::string& offer_sdp) {
    LOG_INFO("Received offer from " + peer_id);

    // Create peer connection
    auto peer = webrtc_manager.CreatePeerConnection(peer_id);

    // Set up callbacks (same as caller side)
    peer->SetOnDataCallback([](const uint8_t* data, size_t size) {
        // Handle data
    });

    peer->SetOnIceCandidateCallback([&](const std::string& candidate) {
        signaling.SendIceCandidate(peer_id, candidate);
    });

    // Initialize
    std::vector<std::string> stun = {"stun:stun.l.google.com:19302"};
    peer->Initialize(stun, {});

    // Set remote description (the offer)
    peer->SetRemoteDescription(offer_sdp);

    // Create and send answer
    std::string answer_sdp;
    if (peer->CreateAnswer(answer_sdp)) {
        signaling.SendAnswer(peer_id, answer_sdp);
        LOG_INFO("Sent answer to " + peer_id);
    }
});
```

### Example 3: Broadcasting to All Peers

```cpp
void BroadcastMovement(uint16_t x, uint16_t y, uint8_t direction) {
    struct MovementPacket {
        uint16_t packet_type = 0x0088; // Movement packet ID
        uint16_t x;
        uint16_t y;
        uint8_t direction;
    } packet = {0x0088, x, y, direction};

    auto peer_ids = webrtc_manager.GetAllPeerIds();
    int sent_count = 0;

    for (const auto& peer_id : peer_ids) {
        auto peer = webrtc_manager.GetPeerConnection(peer_id);
        if (peer && peer->IsConnected()) {
            if (peer->SendData(reinterpret_cast<uint8_t*>(&packet), sizeof(packet))) {
                sent_count++;
            }
        }
    }

    LOG_DEBUG("Broadcast movement to " + std::to_string(sent_count) + " peers");
}
```

---

## Signaling Flow

### Connection Establishment Sequence

```
Client A (Caller)                Signaling Server              Client B (Callee)
      │                                  │                            │
      │  1. CreateOffer()                │                            │
      ├─────────────────────────────────>│                            │
      │     (SDP Offer)                  │                            │
      │                                  │  2. Forward Offer          │
      │                                  ├───────────────────────────>│
      │                                  │                            │
      │                                  │  3. SetRemoteDescription() │
      │                                  │     CreateAnswer()         │
      │                                  │<───────────────────────────┤
      │  4. Forward Answer               │     (SDP Answer)           │
      │<─────────────────────────────────┤                            │
      │                                  │                            │
      │  5. SetRemoteDescription()       │                            │
      │                                  │                            │
      │  6. ICE Candidate Discovery      │                            │
      ├─────────────────────────────────>│  7. Forward ICE            │
      │                                  ├───────────────────────────>│
      │                                  │                            │
      │  8. Forward ICE                  │  9. ICE Candidate Discovery│
      │<─────────────────────────────────┤<───────────────────────────┤
      │                                  │                            │
      │  10. Direct P2P Connection Established                        │
      │<═══════════════════════════════════════════════════════════════>│
      │                                  │                            │
      │  11. Data Channel Open           │                            │
      │  12. SendData() / OnDataCallback │                            │
      │<───────────────────────────────────────────────────────────────>│
```

### State Transitions

```
[Disconnected] ──Initialize()──> [Initialized]
                                      │
                                      │ CreateOffer() / CreateAnswer()
                                      ▼
                                 [Connecting]
                                      │
                                      │ ICE Candidates Exchanged
                                      │ DTLS Handshake Complete
                                      ▼
                                 [Connected]
                                      │
                                      │ Data Channel Open
                                      ▼
                                 [Ready to Send/Receive]
                                      │
                                      │ Connection Lost / Close()
                                      ▼
                                 [Disconnected]
```

---

## STUN/TURN Configuration

### STUN Servers

STUN (Session Traversal Utilities for NAT) servers help discover your public IP address and port.

**Free Public STUN Servers:**

```cpp
std::vector<std::string> stun_servers = {
    "stun:stun.l.google.com:19302",
    "stun:stun1.l.google.com:19302",
    "stun:stun2.l.google.com:19302",
    "stun:stun3.l.google.com:19302",
    "stun:stun4.l.google.com:19302"
};
```

### TURN Servers

TURN (Traversal Using Relays around NAT) servers relay traffic when direct P2P connection fails.

**Configuration with Authentication:**

```cpp
std::vector<std::string> turn_servers = {
    "turn:turn.example.com:3478?transport=udp",
    "turn:turn.example.com:3478?transport=tcp",
    "turns:turn.example.com:5349?transport=tcp" // TLS
};

// Note: libdatachannel supports TURN authentication via URL format:
// "turn:username:password@turn.example.com:3478"
```

### Configuration in p2p_config.json

```json
{
  "webrtc": {
    "stun_servers": [
      "stun:stun.l.google.com:19302",
      "stun:stun1.l.google.com:19302"
    ],
    "turn_servers": ["turn:username:password@your-turn-server.com:3478"],
    "max_peers": 50
  }
}
```

### Setting Up Your Own TURN Server

For production use, deploy your own TURN server using **coturn**:

```bash
# Install coturn on Ubuntu
sudo apt-get install coturn

# Edit /etc/turnserver.conf
listening-port=3478
tls-listening-port=5349
external-ip=YOUR_PUBLIC_IP
realm=your-domain.com
server-name=your-domain.com
lt-cred-mech
user=username:password
```

---

## Testing

### Unit Testing WebRTC Connections

```cpp
// Test peer connection creation
TEST(WebRTCTest, CreatePeerConnection) {
    WebRTCManager manager;
    auto peer = manager.CreatePeerConnection("test_peer");

    ASSERT_NE(peer, nullptr);
    EXPECT_EQ(peer->GetPeerId(), "test_peer");
    EXPECT_FALSE(peer->IsConnected());
}

// Test initialization
TEST(WebRTCTest, Initialize) {
    WebRTCPeerConnection peer("test");
    std::vector<std::string> stun = {"stun:stun.l.google.com:19302"};

    EXPECT_TRUE(peer.Initialize(stun, {}));
}

// Test offer creation
TEST(WebRTCTest, CreateOffer) {
    WebRTCPeerConnection peer("test");
    std::vector<std::string> stun = {"stun:stun.l.google.com:19302"};
    peer.Initialize(stun, {});

    std::string offer;
    EXPECT_TRUE(peer.CreateOffer(offer));
    EXPECT_FALSE(offer.empty());
    EXPECT_TRUE(offer.find("v=0") != std::string::npos); // SDP version
}
```

### Integration Testing

See **[INTEGRATION_TEST_PLAN.md](INTEGRATION_TEST_PLAN.md)** for complete integration testing procedures.

---

## Troubleshooting

### Common Issues

#### Issue: Peer connection never establishes

**Symptoms:**

- `OnStateChangeCallback` never called with `connected=true`
- No data received

**Possible Causes:**

1. **Firewall blocking UDP** - WebRTC uses UDP for data channels
2. **Symmetric NAT** - Requires TURN server
3. **ICE candidates not exchanged** - Check signaling server logs
4. **STUN server unreachable** - Try different STUN servers

**Solutions:**

```cpp
// Enable more verbose logging
LOG_SET_LEVEL(LogLevel::DEBUG);

// Try multiple STUN servers
std::vector<std::string> stun = {
    "stun:stun.l.google.com:19302",
    "stun:stun1.l.google.com:19302",
    "stun:stun.stunprotocol.org:3478"
};

// Add TURN server as fallback
std::vector<std::string> turn = {
    "turn:username:password@your-turn-server.com:3478"
};
```

#### Issue: Data not being received

**Symptoms:**

- Connection established but `OnDataCallback` not triggered

**Possible Causes:**

1. **Data channel not open** - Check `IsConnected()` before sending
2. **Callback not set** - Ensure `SetOnDataCallback()` called before connection
3. **Data size too large** - WebRTC has MTU limits (~16KB per message)

**Solutions:**

```cpp
// Always check connection before sending
if (peer->IsConnected()) {
    peer->SendData(data, size);
} else {
    LOG_WARN("Peer not connected, cannot send data");
}

// Set callback before creating offer/answer
peer->SetOnDataCallback([](const uint8_t* data, size_t size) {
    LOG_DEBUG("Received " + std::to_string(size) + " bytes");
});

// For large data, chunk it
const size_t MAX_CHUNK_SIZE = 16000;
for (size_t offset = 0; offset < total_size; offset += MAX_CHUNK_SIZE) {
    size_t chunk_size = std::min(MAX_CHUNK_SIZE, total_size - offset);
    peer->SendData(data + offset, chunk_size);
}
```

#### Issue: Type conversion errors

**Symptoms:**

- Compilation errors about `std::byte` vs `uint8_t`

**Solution:**
This is already handled in the current implementation. If you see these errors, ensure you're using the latest code.

---

## Performance Tuning

### Optimizing Data Channel Performance

```cpp
// Use unreliable data channel for real-time data (movement, position)
// Note: libdatachannel currently only supports reliable channels
// For unreliable channels, consider using raw UDP with DTLS

// Batch small packets together
std::vector<uint8_t> batch_buffer;
batch_buffer.reserve(16000);

for (const auto& packet : pending_packets) {
    batch_buffer.insert(batch_buffer.end(), packet.begin(), packet.end());
}

if (!batch_buffer.empty()) {
    peer->SendData(batch_buffer.data(), batch_buffer.size());
}
```

### Monitoring Connection Quality

```cpp
// Track connection metrics
struct ConnectionMetrics {
    uint64_t bytes_sent = 0;
    uint64_t bytes_received = 0;
    uint64_t packets_sent = 0;
    uint64_t packets_received = 0;
    std::chrono::steady_clock::time_point last_data_time;
};

std::map<std::string, ConnectionMetrics> peer_metrics;

// Update metrics in callbacks
peer->SetOnDataCallback([&](const uint8_t* data, size_t size) {
    auto& metrics = peer_metrics[peer->GetPeerId()];
    metrics.bytes_received += size;
    metrics.packets_received++;
    metrics.last_data_time = std::chrono::steady_clock::now();
});

// Detect stale connections
void CheckStaleConnections() {
    auto now = std::chrono::steady_clock::now();
    for (const auto& [peer_id, metrics] : peer_metrics) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - metrics.last_data_time).count();

        if (elapsed > 30) { // 30 seconds timeout
            LOG_WARN("Peer " + peer_id + " appears stale, reconnecting...");
            webrtc_manager.RemovePeerConnection(peer_id);
        }
    }
}
```

---

## Additional Resources

- **libdatachannel Documentation:** https://github.com/paullouisageneau/libdatachannel
- **WebRTC Specification:** https://www.w3.org/TR/webrtc/
- **ICE RFC:** https://tools.ietf.org/html/rfc8445
- **STUN RFC:** https://tools.ietf.org/html/rfc5389
- **TURN RFC:** https://tools.ietf.org/html/rfc5766

---

**WebRTC Guide Version:** 1.0
**Last Updated:** November 8, 2025
**Maintained by:** rAthena AI World Development Team
