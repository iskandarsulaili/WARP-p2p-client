# WARP P2P Client - WebSocket Integration Implementation Guide

**Date**: 2025-11-07  
**Status**: Reference Implementation  
**Target**: Ragnarok Online Client (C++)

---

## Important Notice

**WARP is a patcher tool**, not the actual game client. This guide provides reference implementations for integrating WebSocket-based P2P coordinator signaling into the **actual Ragnarok Online client executable**.

The files in this directory are **reference implementations** that show how to modify the RO client to connect to the rathena-AI-world P2P coordinator service.

---

## Architecture Overview

```
Ragnarok Online Client (C++)
├── Core/Network/P2PNetwork.hpp/cpp     ← WebSocket client implementation
├── Core/Network/P2PSession.hpp/cpp     ← Session management
├── Core/Game/MapSystem_P2P.cpp         ← Map system integration
└── Data/config.ini                     ← Configuration
         ↓ WebSocket Connection
    ws://localhost:8001/api/signaling/ws
         ↓
rathena-AI-world P2P Coordinator (Python/FastAPI)
├── WebSocket Signaling Server
├── Session Manager
└── Host Registry
```

---

## Phase 1: Add WebSocket Dependencies

### CMakeLists.txt Modifications

Add to the RO client's CMakeLists.txt:

```cmake
# P2P WebSocket Support
option(ENABLE_P2P "Enable P2P hosting support" ON)

if(ENABLE_P2P)
    add_definitions(-DENABLE_P2P_SUPPORT)
    
    # Find WebSocket++ (header-only library)
    find_path(WEBSOCKETPP_INCLUDE_DIR
        NAMES websocketpp/config/asio_client.hpp
        PATHS /usr/include /usr/local/include
    )
    
    # Find nlohmann/json (header-only library)
    find_path(JSON_INCLUDE_DIR
        NAMES nlohmann/json.hpp
        PATHS /usr/include /usr/local/include
    )
    
    # Find Asio (required by WebSocket++)
    find_package(Asio REQUIRED)
    
    # Find OpenSSL (already required)
    find_package(OpenSSL REQUIRED)
    
    # Add P2P source files
    set(P2P_SOURCES
        Core/Network/P2PNetwork.cpp
        Core/Network/P2PSession.cpp
        Core/Game/MapSystem_P2P.cpp
    )
    
    # Add to client target
    target_sources(client PRIVATE ${P2P_SOURCES})
    
    # Include directories
    target_include_directories(client PRIVATE
        ${WEBSOCKETPP_INCLUDE_DIR}
        ${JSON_INCLUDE_DIR}
        ${ASIO_INCLUDE_DIR}
    )
    
    # Link libraries
    target_link_libraries(client PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
    )
    
    message(STATUS "P2P WebSocket support enabled")
endif()
```

### Installing Dependencies (Linux)

```bash
# WebSocket++ (header-only)
sudo apt-get install libwebsocketpp-dev

# nlohmann/json (header-only)
sudo apt-get install nlohmann-json3-dev

# Asio (header-only)
sudo apt-get install libasio-dev

# OpenSSL (already installed for RO client)
sudo apt-get install libssl-dev
```

### Installing Dependencies (Windows)

```powershell
# Using vcpkg
vcpkg install websocketpp:x64-windows
vcpkg install nlohmann-json:x64-windows
vcpkg install asio:x64-windows
vcpkg install openssl:x64-windows
```

---

## Phase 2: Implement P2PNetwork Class

See `Core/Network/P2PNetwork.hpp` and `Core/Network/P2PNetwork.cpp` for full implementation.

### Key Features

1. **WebSocket Connection Management**
   - Connect to coordinator at `ws://localhost:8001/api/signaling/ws`
   - Automatic reconnection with exponential backoff
   - Graceful disconnection handling

2. **Message Handling**
   - JSON serialization/deserialization
   - Message types: join, leave, offer, answer, ice-candidate
   - Event callbacks for application integration

3. **Session Management**
   - Join/leave sessions with session_id
   - Track connected peers
   - Handle peer join/leave events

4. **WebRTC Integration**
   - Forward SDP offers/answers to WebRTC engine
   - Handle ICE candidate exchange
   - Trigger P2P connection establishment

---

## Phase 3: Configuration Updates

### Data/config.ini

```ini
[P2P]
; Enable P2P hosting
EnableP2P=1
PreferP2PHosting=1
MainServerFallback=1

; Coordinator connection
CoordinatorURL=ws://localhost:8001/api/signaling/ws
CoordinatorPort=8001
ConnectionTimeout=5000
ReconnectDelay=1000
MaxRetries=3

; Host Requirements
MinCPUCores=4
MinCPUFrequency=3000
MinMemoryMB=8192
MinNetworkSpeedMbps=100
MaxLatencyMS=100

; Security
EnableEncryption=1
KeyRotationInterval=3600

; Performance
UpdateInterval=50
MaxConnections=5
CompressionThreshold=1024
```

### Data/resourceinfo.ini

```ini
[P2P_HOSTS]
; Primary coordinator (development)
0=ws://localhost:8001/api/signaling/ws

; Production coordinator (update when deployed)
; 0=wss://coordinator.yourdomain.com/api/signaling/ws

; Backup coordinator
; 1=wss://backup-coordinator.yourdomain.com/api/signaling/ws
```

---

## Phase 4: Integration with Map System

### Core/Game/MapSystem_P2P.cpp

```cpp
#include "MapSystem.hpp"
#include "P2PNetwork.hpp"
#include "Config.hpp"

void MapSystem::initialize_p2p() {
    if (!Config::get_bool("P2P", "EnableP2P")) {
        return;
    }
    
    std::string coordinator_url = Config::get_string("P2P", "CoordinatorURL");
    std::string peer_id = generate_peer_id();
    
    p2p_network_ = std::make_unique<P2PNetwork>();
    
    // Set up event handlers
    p2p_network_->on_session_joined([this](const std::vector<std::string>& peers) {
        handle_session_joined(peers);
    });
    
    p2p_network_->on_peer_joined([this](const std::string& peer_id) {
        handle_peer_joined(peer_id);
    });
    
    p2p_network_->on_offer_received([this](const std::string& from, const json& sdp) {
        handle_webrtc_offer(from, sdp);
    });
    
    // Connect to coordinator
    if (!p2p_network_->connect(coordinator_url, peer_id)) {
        logger.error("Failed to connect to P2P coordinator");
        // Fall back to main server
        use_main_server();
    }
}
```

---

## Testing Checklist

- [ ] WebSocket connection establishes successfully
- [ ] Client sends join message with peer_id and session_id
- [ ] Client receives session-joined with peer list
- [ ] Client creates WebRTC offers to existing peers
- [ ] Client handles incoming offers from new peers
- [ ] ICE candidates are exchanged correctly
- [ ] WebRTC P2P connection establishes
- [ ] Fallback to main server works when coordinator unavailable
- [ ] Reconnection works after network interruption
- [ ] Multiple peers can join same session

---

## Next Steps

1. Review reference implementation files in `Core/Network/`
2. Integrate into actual RO client build system
3. Test with rathena-AI-world P2P coordinator
4. Deploy to production with WSS (secure WebSocket)


