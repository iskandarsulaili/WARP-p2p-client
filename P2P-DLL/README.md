# P2P Network DLL

**Version**: 1.0.0
**Status**: Phase 1 - Foundation (In Development)
**Location**: `WARP-p2p-client/P2P-DLL/`

---

## Overview

Production-ready P2P networking DLL for Ragnarok Online client integration. Provides WebRTC-based peer-to-peer connections for zone-based gameplay while maintaining centralized AI NPCs, authentication, and anti-cheat systems.

**Architecture**: Separate DLL injected into RO client via NEMO patcher

**Integration**: This P2P DLL is part of the WARP-p2p-client package, which includes:
- **NEMO Patcher** - Tool to patch RO client executables
- **P2P-DLL** - This P2P networking implementation (C++)
- **LoadP2PDLL.qs** - NEMO patch script to inject the DLL

---

## Features

### Implemented (Phase 1)
- ✅ Project structure and build system
- ✅ Configuration management (JSON)
- ✅ Logging infrastructure (spdlog)
- 🚧 WebSocket signaling client (in progress)
- 🚧 ConfigManager implementation (in progress)

### Planned
- 📋 Production WebRTC integration (libwebrtc M120+)
- 📋 Packet routing with function hooking
- 📋 JWT authentication
- 📋 AES-256-GCM encryption
- 📋 Session recovery
- 📋 NEMO patch for DLL injection

---

## Prerequisites

### Development Environment
- **OS**: Windows 10/11
- **Compiler**: MSVC 2022 (C++17)
- **Build Tool**: CMake 3.25+
- **Package Manager**: vcpkg

### Dependencies (via vcpkg)
```bash
vcpkg install nlohmann-json:x64-windows
vcpkg install spdlog:x64-windows
vcpkg install openssl:x64-windows
vcpkg install websocketpp:x64-windows
vcpkg install cpp-httplib:x64-windows
vcpkg install gtest:x64-windows
vcpkg install detours:x64-windows
```

### Manual Dependencies
- **libwebrtc M120+**: See `docs/WEBRTC_INTEGRATION.md` (to be created)

---

## Build Instructions

### 1. Navigate to P2P-DLL Directory
```bash
cd /path/to/ai-mmorpg-world/WARP-p2p-client/P2P-DLL
```

### 2. Install Dependencies
```bash
# Set vcpkg root (if not already set)
export VCPKG_ROOT=/path/to/vcpkg

# Install dependencies
vcpkg install nlohmann-json spdlog openssl websocketpp cpp-httplib gtest detours --triplet x64-windows
```

### 3. Configure
```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
```

### 4. Build
```bash
cmake --build build --config Release
```

### 5. Run Tests
```bash
cd build
ctest -C Release --output-on-failure
```

### 6. Package
```bash
cmake --build build --target package
```

**Output**: `build/P2PNetworkDLL-1.0.0-win64.zip`

---

## Project Structure

```
WARP-p2p-client/P2P-DLL/
├── CMakeLists.txt           # Main CMake configuration
├── vcpkg.json               # vcpkg manifest
├── README.md                # This file
├── config/
│   └── p2p_config.json      # Default configuration
├── include/                 # Public headers
│   ├── NetworkManager.h
│   ├── ConfigManager.h
│   ├── SignalingClient.h
│   ├── PacketRouter.h
│   ├── WebRTCManager.h
│   ├── WebRTCPeerConnection.h
│   ├── SecurityManager.h
│   ├── Logger.h
│   └── Types.h
├── src/                     # Implementation
│   ├── core/
│   │   ├── NetworkManager.cpp
│   │   └── ConfigManager.cpp
│   ├── network/
│   │   ├── SignalingClient.cpp
│   │   └── PacketRouter.cpp
│   ├── webrtc/
│   │   ├── WebRTCManager.cpp
│   │   └── WebRTCPeerConnection.cpp
│   ├── security/
│   │   └── SecurityManager.cpp
│   ├── utils/
│   │   └── Logger.cpp
│   └── DllMain.cpp          # DLL entry point
├── tests/                   # Unit tests
│   ├── CMakeLists.txt
│   ├── test_config.cpp
│   ├── test_signaling.cpp
│   └── test_webrtc.cpp
├── docs/                    # Documentation
│   ├── API.md
│   ├── ARCHITECTURE.md
│   └── WEBRTC_INTEGRATION.md
└── cmake/                   # CMake modules
    └── FindWebRTC.cmake
```

---

## Configuration

Edit `config/p2p_config.json` to configure the DLL:

```json
{
  "coordinator": {
    "rest_api_url": "http://localhost:8001/api/v1",
    "websocket_url": "ws://localhost:8001/api/v1/signaling/ws"
  },
  "p2p": {
    "enabled": true,
    "max_peers": 50
  },
  "logging": {
    "level": "info",
    "file": "p2p_dll.log"
  }
}
```

---

## Usage

### 1. Build DLL
```bash
cmake --build build --config Release
```

### 2. Copy to RO Directory
```bash
cp build/bin/Release/p2p_network.dll /path/to/RagnarokOnline/
cp config/p2p_config.json /path/to/RagnarokOnline/
```

### 3. Patch RO Client with NEMO
- Open NEMO patcher (`WARP-p2p-client/NEMO.exe`)
- Load RO client executable
- Apply "Load P2P DLL" patch (located in `WARP-p2p-client/Patches/LoadP2PDLL.qs`)
- Save patched client to Output directory

### 4. Launch Client
```bash
cd /path/to/RagnarokOnline
./Ragnarok.exe
```

### 5. Check Logs
```bash
tail -f /path/to/RagnarokOnline/logs/p2p_dll.log
```

---

## Development Status

See `P2P_DLL_BUILD_PLAN.md` in project root for complete roadmap.

**Current Phase**: Phase 1 - Foundation (Week 1-2)

**Progress**:
- [x] Project structure created
- [x] CMake build system configured
- [x] Configuration file created
- [ ] ConfigManager implementation
- [ ] Logging infrastructure
- [ ] WebSocket signaling client
- [ ] Unit tests

---

## Related Documentation

- `../../P2P_DLL_BUILD_PLAN.md` - Complete build plan (1,228 lines)
- `../../COORDINATOR_DLL_INTEGRATION.md` - Integration analysis (1,274 lines)
- `../../P2P_FEATURES.md` - P2P features documentation
- `../../rathena-AI-world/p2p-coordinator/docs/API.md` - Coordinator API reference
- `../Patches/LoadP2PDLL.qs` - NEMO patch script for DLL injection

---

## License

Part of the rAthena AI autonomous world system.

---

## Support

For issues and questions:
- Check documentation in `docs/`
- Review logs: `logs/p2p_dll.log`
- See troubleshooting guide: `docs/TROUBLESHOOTING.md`


