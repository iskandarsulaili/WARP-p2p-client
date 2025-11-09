# P2P Network DLL - Deployment Guide

**Version:** 2.0.0
**Last Updated:** November 9, 2025
**Platform:** Windows 10/11 (x64)
**Status:** ✅ Production-Ready - All 26 Security & Functionality Fixes Complete

---

## 🎉 What's New in Version 2.0.0

**All 26 critical fixes have been completed and are production-ready:**

✅ **Critical Fixes (6/6)**:
- Fixed authentication race condition with synchronous auth
- Implemented packet serialization/deserialization with CRC32 validation
- Completed packet routing with automatic fallback
- Implemented WebSocket signaling with auto-reconnection
- Fixed session ID type mismatches (UUID-based)
- Moved signaling state to Redis for persistence

✅ **High Severity Fixes (6/6)**:
- Enabled SSL certificate verification
- Removed hardcoded security secrets (environment-based)
- Implemented JWT token parsing and refresh
- Completed session discovery and joining
- **NEW**: RO client packet hooking via NEMO patch
- **NEW**: WebRTC offer/answer/ICE candidate flow

✅ **Medium Severity Fixes (8/8)**:
- Connection recovery with exponential backoff
- Fixed deprecated datetime functions
- NPC state broadcasting every 5 seconds
- Session health monitoring with auto-cleanup
- Rate limiting (token bucket algorithm)
- Fallback mechanism to server routing
- Evaluated synchronous HTTP (acceptable for infrequent ops)
- Replaced sleep-based async with condition variables

✅ **Low Severity Fixes (6/6)**:
- Removed duplicate config fields
- Added database indexes for performance
- Implemented Prometheus metrics
- Proper error handling with custom exceptions
- Fixed WebSocket send error handling
- Implemented refresh token endpoint

---

## Table of Contents

1. [Overview](#overview)
2. [Quick Start Guide](#quick-start-guide)
3. [Prerequisites](#prerequisites)
4. [Complete Deployment Workflow](#complete-deployment-workflow)
5. [NEMO Patcher Integration](#nemo-patcher-integration)
6. [DLL Dependencies](#dll-dependencies)
7. [Configuration Setup](#configuration-setup)
8. [Zone-Based P2P and Graceful Fallback](#zone-based-p2p-and-graceful-fallback)
9. [Coordinator Server Deployment](#coordinator-server-deployment)
10. [Security Considerations](#security-considerations)
11. [Testing Deployment](#testing-deployment)
12. [Troubleshooting](#troubleshooting)

---

## Overview

This guide explains how to deploy the P2P Network DLL to end-users and integrate it with the Ragnarok Online client using NEMO patcher.

### ⚠️ Important: P2P is Completely Optional

**The P2P system is entirely optional and can be disabled at any time:**
- When P2P is disabled or unavailable, the system **automatically falls back** to traditional server routing
- Players experience **no difference in gameplay** when P2P is disabled
- The fallback is **transparent** - no manual intervention required
- You can enable/disable P2P per zone or globally via configuration

**P2P provides benefits when enabled:**
- Reduced server load in high-traffic zones
- Lower latency for player-to-player interactions
- Distributed bandwidth usage

**But the game works perfectly without it** - P2P is a performance enhancement, not a requirement.

### Deployment Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     End User's PC                            │
│  ┌───────────────────────────────────────────────────────┐  │
│  │  Ragnarok Online Client (Patched with NEMO)          │  │
│  │  ┌─────────────────────────────────────────────────┐ │  │
│  │  │  p2p_network.dll (Injected)                     │ │  │
│  │  │  + Dependencies (OpenSSL, spdlog, etc.)         │ │  │
│  │  │  + p2p_config.json                              │ │  │
│  │  └─────────────────────────────────────────────────┘ │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                          │
                          │ HTTPS/WSS
                          ▼
┌─────────────────────────────────────────────────────────────┐
│                  Coordinator Server                          │
│  ┌───────────────────────────────────────────────────────┐  │
│  │  REST API (Port 443)                                  │  │
│  │  WebSocket Signaling (Port 443)                       │  │
│  │  Authentication & Session Management                  │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                          │
                          │ (Optional)
                          ▼
┌─────────────────────────────────────────────────────────────┐
│                  STUN/TURN Servers                           │
│  ┌───────────────────────────────────────────────────────┐  │
│  │  STUN: UDP 3478, 19302                                │  │
│  │  TURN: UDP/TCP 3478, TLS 5349                         │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

---

## Quick Start Guide

**🚀 Got a coordinator server URL and want to deploy quickly? Follow these steps:**

### What You Need

- ✅ Built `p2p_network.dll` and dependencies (in `P2P-DLL/build/bin/Release/`)
- ✅ NEMO.exe patcher (in workspace root)
- ✅ Ragnarok Online client executable
- ✅ Coordinator server URL and API key

### 5-Minute Deployment

**Step 1: Configure P2P Settings** (2 minutes)

```powershell
# Edit the configuration file
notepad P2P-DLL\config\p2p_config.json
```

Update these critical values:

```json
{
  "coordinator": {
    "rest_api_url": "https://YOUR-COORDINATOR-URL.com/api/v1",
    "websocket_url": "wss://YOUR-COORDINATOR-URL.com/api/v1/signaling/ws"
  },
  "security": {
    "api_key": "YOUR-API-KEY-HERE"
  }
}
```

**Step 2: Deploy DLL Files** (1 minute)

```powershell
# Copy ALL DLLs to your RO directory
$RO_DIR = "C:\Path\To\RagnarokOnline"
Copy-Item P2P-DLL\build\bin\Release\*.dll $RO_DIR
Copy-Item P2P-DLL\config\p2p_config.json $RO_DIR
```

**Step 3: Patch RO Client with NEMO** (2 minutes)

```powershell
# Launch NEMO
.\NEMO.exe
```

1. Click "Load Client" → Select your RO executable
2. Find and enable "Load P2P Network DLL" patch
3. Click "Apply Selected Patches"
4. Save as `Ragnarok_P2P.exe`

**Step 4: Test the Integration**

```powershell
# Run the patched client
cd $RO_DIR
.\Ragnarok_P2P.exe
```

Check for `p2p_dll.log` in the RO directory. Should show:

```
[info] P2P Network DLL loaded successfully
[info] P2P networking is ENABLED
```

**✅ Done!** Your P2P-enabled client is ready.

**⚠️ For end users:** They need [Visual C++ Redistributable 2022](https://aka.ms/vs/17/release/vc_redist.x64.exe) installed.

---

## Prerequisites

### For Deployment

- **NEMO Patcher** - Latest version from https://gitlab.com/4144/Nemo
- **Ragnarok Online Client** - Compatible client executable (2018+ recommended)
- **Built DLL** - `p2p_network.dll` from [BUILD_GUIDE.md](BUILD_GUIDE.md)
- **Coordinator Server** - Running and accessible (see [Coordinator Server Deployment](#coordinator-server-deployment))

### For End Users

- **Windows 10/11** (64-bit)
- **Visual C++ Redistributable 2022** (x64) - https://aka.ms/vs/17/release/vc_redist.x64.exe
- **Internet Connection** - For coordinator and STUN/TURN servers
- **Firewall Configuration** - Allow UDP traffic for WebRTC

---

## Complete Deployment Workflow

**This section provides a detailed, step-by-step workflow for deploying the P2P system to production.**

### Prerequisites Checklist

Before starting, ensure you have:

- [ ] Built `p2p_network.dll` in Release mode
- [ ] All dependency DLLs available (9 DLLs total - see [DLL Dependencies](#dll-dependencies))
- [ ] NEMO patcher (`NEMO.exe`)
- [ ] Ragnarok Online client executable
- [ ] Coordinator server URL (e.g., `https://coordinator.example.com`)
- [ ] API key from coordinator server
- [ ] (Optional) TURN server credentials if behind strict NAT

### Phase 1: Configuration (5 minutes)

**1.1 Locate the Configuration File**

```powershell
cd P2P-DLL\config
notepad p2p_config.json
```

**1.2 Update Coordinator URLs**

Replace `localhost` with your actual coordinator server:

```json
{
  "coordinator": {
    "rest_api_url": "https://coordinator.example.com/api/v1",
    "websocket_url": "wss://coordinator.example.com/api/v1/signaling/ws",
    "timeout_seconds": 30,
    "reconnect_max_attempts": 5,
    "reconnect_backoff_ms": 1000
  }
}
```

**1.3 Set API Key**

Add your coordinator API key:

```json
{
  "security": {
    "enable_encryption": true,
    "enable_authentication": true,
    "api_key": "your-actual-api-key-here",
    "jwt_token": "",
    "certificate_validation": true,
    "tls_version": "1.3"
  }
}
```

**1.4 Configure P2P Zones (Optional)**

Customize which zones use P2P:

```json
{
  "zones": {
    "p2p_enabled_zones": [
      "prontera",
      "geffen",
      "payon",
      "morocc",
      "alberta",
      "aldebaran",
      "izlude"
    ],
    "fallback_on_failure": true,
    "zone_transition_timeout_ms": 5000
  }
}
```

**1.5 Adjust Logging (Optional)**

For production, use `"info"` level. For debugging, use `"debug"`:

```json
{
  "logging": {
    "level": "info",
    "file": "p2p_dll.log",
    "max_file_size_mb": 10,
    "max_files": 5,
    "console_output": false,
    "async_logging": true
  }
}
```

**1.6 Validate Configuration**

Ensure the JSON is valid:

```powershell
# Test JSON syntax
Get-Content p2p_config.json | ConvertFrom-Json
```

### Phase 2: DLL Deployment (3 minutes)

**2.1 Prepare Deployment Directory**

```powershell
# Set your RO directory
$RO_DIR = "C:\RagnarokOnline"

# Create config directory if needed
New-Item -ItemType Directory -Force -Path "$RO_DIR"
```

**2.2 Copy All DLL Files**

```powershell
# Copy ALL 9 DLLs from build directory
Copy-Item "P2P-DLL\build\bin\Release\*.dll" -Destination $RO_DIR -Force

# Verify all DLLs copied
Get-ChildItem $RO_DIR -Filter "*.dll" | Select-Object Name, Length
```

**Expected output:**

```
Name                    Length
----                    ------
p2p_network.dll         515584
libssl-3-x64.dll        871424
libcrypto-3-x64.dll     5327872
spdlog.dll              285696
brotlicommon.dll        137728
brotlidec.dll           52224
fmt.dll                 120832
gtest.dll               454144
gtest_main.dll          74240
```

**2.3 Copy Configuration File**

```powershell
# Copy config to RO directory
Copy-Item "P2P-DLL\config\p2p_config.json" -Destination $RO_DIR -Force
```

**2.4 Verify Deployment**

```powershell
# Check all required files are present
$required = @(
    "p2p_network.dll",
    "libssl-3-x64.dll",
    "libcrypto-3-x64.dll",
    "spdlog.dll",
    "brotlicommon.dll",
    "brotlidec.dll",
    "fmt.dll",
    "p2p_config.json"
)

foreach ($file in $required) {
    if (Test-Path "$RO_DIR\$file") {
        Write-Host "✅ $file" -ForegroundColor Green
    } else {
        Write-Host "❌ $file MISSING" -ForegroundColor Red
    }
}
```

### Phase 3: NEMO Patching (5 minutes)

**3.1 Launch NEMO Patcher**

```powershell
# From workspace root
.\NEMO.exe
```

**3.2 Load RO Client**

1. Click **"Load Client"** button
2. Navigate to your RO directory
3. Select your client executable (e.g., `Ragnarok.exe`, `2020-04-01aRagexe.exe`)
4. Wait for NEMO to analyze the client

**3.3 Select P2P Patch**

1. In the patch list, search for **"Load P2P Network DLL"**
2. Check the box to enable it
3. (Optional) Select other patches you need

**3.4 Apply Patches**

1. Click **"Apply Selected Patches"** button
2. Wait for patching to complete
3. When prompted, save the patched client as `Ragnarok_P2P.exe`

**3.5 Verify Patch Success**

NEMO should show:

```
✅ Patch "Load P2P Network DLL" applied successfully
✅ Client saved as: Ragnarok_P2P.exe
```

### Phase 4: Testing (10 minutes)

**4.1 Test with P2P Disabled First**

Edit `p2p_config.json` temporarily:

```json
{
  "p2p": {
    "enabled": false
  }
}
```

Run the client:

```powershell
cd $RO_DIR
.\Ragnarok_P2P.exe
```

**Expected:** Client launches normally, log shows:

```
[info] P2P Network DLL loaded successfully
[info] P2P networking is DISABLED in configuration
```

**4.2 Test with P2P Enabled**

Re-enable P2P:

```json
{
  "p2p": {
    "enabled": true
  }
}
```

Run the client again and check logs:

```powershell
# View log file
Get-Content p2p_dll.log -Tail 20
```

**Expected log output:**

```
[info] P2P Network DLL loaded successfully
[info] P2P networking is ENABLED
[info] Coordinator URL: https://coordinator.example.com/api/v1
[info] Authenticating with coordinator...
[info] Authentication successful
```

**4.3 Test Zone Transition**

1. Log into the game
2. Enter a P2P-enabled zone (e.g., Prontera)
3. Check logs for zone change:

```
[info] Zone changed to: prontera
[info] P2P enabled for zone: prontera
[info] Connecting to signaling server...
```

**4.4 Verify Log Files**

Check that log files are being created:

```powershell
Get-ChildItem $RO_DIR -Filter "p2p_dll*.log"
```

Should show:

```
p2p_dll.log       (current log)
p2p_dll.1.log     (rotated log)
```

### Phase 5: End-User Distribution

**5.1 Prepare Distribution Package**

Create a package for end users:

```powershell
# Create distribution folder
$DIST_DIR = "RO_P2P_Client_v1.0"
New-Item -ItemType Directory -Force -Path $DIST_DIR

# Copy patched client
Copy-Item "$RO_DIR\Ragnarok_P2P.exe" -Destination $DIST_DIR

# Copy DLLs
Copy-Item "$RO_DIR\*.dll" -Destination $DIST_DIR

# Copy config
Copy-Item "$RO_DIR\p2p_config.json" -Destination $DIST_DIR

# Copy VC++ Redistributable installer
Invoke-WebRequest -Uri "https://aka.ms/vs/17/release/vc_redist.x64.exe" -OutFile "$DIST_DIR\vc_redist.x64.exe"
```

**5.2 Create Installation Instructions**

Create `INSTALL.txt` in the distribution folder:

```text
P2P-Enabled Ragnarok Online Client - Installation Guide

REQUIREMENTS:
- Windows 10/11 (64-bit)
- Internet connection
- Firewall allowing UDP traffic

INSTALLATION STEPS:

1. Install Visual C++ Redistributable:
   - Run vc_redist.x64.exe
   - Follow the installation wizard

2. Copy all files to your Ragnarok Online directory:
   - Ragnarok_P2P.exe
   - All .dll files
   - p2p_config.json

3. Run Ragnarok_P2P.exe to start the game

TROUBLESHOOTING:

- If client crashes on startup:
  → Reinstall vc_redist.x64.exe
  → Check that all .dll files are present

- If P2P doesn't work:
  → Check p2p_dll.log for errors
  → Ensure firewall allows UDP traffic

SUPPORT:
Email: support@example.com
Discord: [Your Discord Server]
```

**5.3 Package for Distribution**

```powershell
# Create ZIP archive
Compress-Archive -Path $DIST_DIR -DestinationPath "RO_P2P_Client_v1.0.zip"
```

### Phase 6: Verification Checklist

Before distributing to users, verify:

- [ ] Patched client launches without errors
- [ ] Log file `p2p_dll.log` is created
- [ ] P2P DLL loads successfully (check log)
- [ ] Coordinator connection succeeds (check log)
- [ ] Zone transitions work correctly
- [ ] P2P-enabled zones show peer connections
- [ ] Non-P2P zones fall back to server
- [ ] Client works when P2P is disabled
- [ ] All DLL dependencies are included
- [ ] VC++ Redistributable installer is included
- [ ] Installation instructions are clear

---

## NEMO Patcher Integration

### Step 1: Prepare NEMO Patch Scripts

The P2P system uses **two NEMO patches**:

1. **`LoadP2PDLL.qs`** - Injects P2P DLL into RO client at startup
2. **`HookP2PPackets.qs`** - Hooks send/recv functions for packet interception (NEW in v2.0.0)

**Location:** `Patches/LoadP2PDLL.qs` and `Patches/HookP2PPackets.qs`

**LoadP2PDLL.qs Overview:**

```javascript
function LoadP2PDLL() {
  // Finds RO client entry point
  // Injects LoadLibrary("p2p_network.dll") call
  // Stores DLL handle for later use
}
```

**HookP2PPackets.qs Overview (NEW):**

```javascript
function HookP2PPackets() {
  // Hooks Winsock send() and recv() functions
  // Intercepts game packets for P2P routing
  // Calls P2P_RoutePacket() DLL export function
  // Automatically falls back to server if P2P unavailable
}
```

### Step 2: Add Patches to NEMO

1. **Copy both patch scripts** to NEMO's `Patches/` directory:

   ```
   NEMO/
   ├── Patches/
   │   ├── LoadP2PDLL.qs       ← Copy here
   │   └── HookP2PPackets.qs   ← Copy here (NEW)
   ```

2. **Register patches** in NEMO's patch list (if not auto-detected)

3. **Verify both patches appear** in NEMO's patch selection UI

### Step 3: Patch RO Client

1. **Launch NEMO Patcher**

   ```powershell
   cd NEMO
   .\Nemo.exe
   ```

2. **Load RO client executable**

   - Click "Load Client"
   - Select your RO client (e.g., `Ragnarok.exe`, `2018-06-20aRagexe.exe`)

3. **Select patches**

   - ✅ Enable "Load P2P Network DLL" (Required)
   - ✅ Enable "Hook P2P Packets" (NEW - Required for packet routing)
   - ✅ Enable other desired patches (e.g., "Disable 1rag1 type parameters", "Enable Multiple GRFs")

4. **Apply patches**

   - Click "Apply Selected Patches"
   - Save patched client (e.g., `Ragnarok_P2P.exe`)

5. **Verify patches**
   - Check NEMO output for success messages:
     ```
     ✅ Patch "Load P2P Network DLL" applied successfully
     ✅ Patch "Hook P2P Packets" applied successfully
     ```
   - Patched client should be created

**Note**: Both patches are required for full P2P functionality. The "Hook P2P Packets" patch enables automatic packet routing through the P2P system.

### Step 4: Deploy Patched Client

**Directory Structure:**

```
RagnarokOnline/
├── Ragnarok_P2P.exe          ← Patched client
├── p2p_network.dll           ← P2P DLL
├── config/
│   └── p2p_config.json       ← Configuration
├── logs/                     ← Log directory (auto-created)
├── [OpenSSL DLLs]            ← Dependencies (see next section)
├── [Other RO files]
```

---

## DLL Dependencies

### Required DLL Files

The P2P DLL requires the following dependencies to be present in the same directory as the RO client:

| DLL File              | Size   | Source          | Purpose                        | Required    |
| --------------------- | ------ | --------------- | ------------------------------ | ----------- |
| `p2p_network.dll`     | ~600KB | Build output    | Main P2P DLL with all 26 fixes | ✅ Yes      |
| `libssl-3-x64.dll`    | 871 KB | OpenSSL 3.0+    | SSL/TLS with cert verification | ✅ Yes      |
| `libcrypto-3-x64.dll` | 5.3 MB | OpenSSL 3.0+    | Cryptography (AES-256-GCM)     | ✅ Yes      |
| `spdlog.dll`          | 285 KB | vcpkg           | Async logging framework        | ✅ Yes      |
| `brotlicommon.dll`    | 137 KB | vcpkg           | Compression (Boost dependency) | ✅ Yes      |
| `brotlidec.dll`       | 52 KB  | vcpkg           | Compression (Boost dependency) | ✅ Yes      |
| `fmt.dll`             | 120 KB | vcpkg           | String formatting              | ✅ Yes      |
| `libdatachannel.dll`  | ~2 MB  | vcpkg           | WebRTC implementation          | ✅ Yes      |
| `gtest.dll`           | 454 KB | vcpkg           | Testing framework              | ⚠️ Optional |
| `gtest_main.dll`      | 74 KB  | vcpkg           | Testing framework              | ⚠️ Optional |

**Total Size:** ~10 MB (excluding optional test DLLs)

**Note:** `gtest.dll` and `gtest_main.dll` are only needed for development/testing. They can be excluded from production deployments.

### New in Version 2.0.0

- **OpenSSL 3.0+**: Now enforces SSL certificate verification (no more `ssl::verify_none`)
- **libdatachannel**: Added for WebRTC offer/answer/ICE candidate flow
- **Condition Variables**: Replaced sleep-based async waiting for better performance

### Obtaining Dependencies

**Option 1: Copy from Build Directory**

After building the DLL, copy dependencies from the build output:

```powershell
# From P2P-DLL/build/bin/Release/
Copy-Item "libssl-3-x64.dll" -Destination "C:\RagnarokOnline\"
Copy-Item "libcrypto-3-x64.dll" -Destination "C:\RagnarokOnline\"
Copy-Item "spdlog.dll" -Destination "C:\RagnarokOnline\"
Copy-Item "brotlidec.dll" -Destination "C:\RagnarokOnline\"
```

**Option 2: Use Dependency Walker**

Use [Dependencies](https://github.com/lucasg/Dependencies) to identify and copy all required DLLs:

```powershell
# Download Dependencies.exe
# Open p2p_network.dll
# View "Module List" to see all required DLLs
```

### Visual C++ Redistributable

End users must install **Visual C++ Redistributable 2022 (x64)**:

**Download:** https://aka.ms/vs/17/release/vc_redist.x64.exe

**Installation:**

```powershell
# Silent install
vc_redist.x64.exe /install /quiet /norestart
```

---

## Configuration Setup

### Production Configuration Template

Create `config/p2p_config.json` in the RO client directory:

```json
{
  "coordinator": {
    "rest_api_url": "https://your-server.com/api/v1",
    "websocket_url": "wss://your-server.com/api/v1/signaling/ws",
    "timeout_ms": 5000,
    "reconnect_max_attempts": 5,
    "reconnect_backoff_ms": 1000
  },
  "webrtc": {
    "stun_servers": [
      "stun:stun.l.google.com:19302",
      "stun:stun1.l.google.com:19302"
    ],
    "turn_servers": ["turn:username:password@your-turn-server.com:3478"]
  },
  "p2p": {
    "enabled": true,
    "max_peers": 50
  },
  "security": {
    "enable_encryption": true,
    "enable_authentication": true,
    "api_key": "YOUR_API_KEY_HERE_MINIMUM_32_CHARACTERS",
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
    "fallback_on_failure": true
  }
}
```

### Configuration Security (NEW in v2.0.0)

**⚠️ CRITICAL SECURITY REQUIREMENTS:**

1. **API Keys Must Be 32+ Characters**: The coordinator service now enforces minimum 32-character secrets in production
2. **SSL Certificate Verification Enabled**: `certificate_validation` is now enforced - no more `ssl::verify_none`
3. **Environment-Based Secrets**: Hardcoded secrets have been removed from the codebase

**Sensitive Fields:**

- `security.api_key` - **MUST be 32+ characters** (enforced by coordinator)
- `security.jwt_token` - Generated at runtime, auto-refreshed
- `turn_servers` - May contain credentials

**Best Practices:**

1. **Generate Strong API Keys**: Use `openssl rand -hex 32` to generate 64-character keys
2. **Use Environment Variables**: Never commit secrets to version control
3. **Rotate Credentials Regularly**: Implement 90-day rotation policy
4. **Use HTTPS/WSS Only**: HTTP/WS are rejected in production
5. **Enable Certificate Validation**: Always set `certificate_validation: true`
6. **Use TLS 1.3**: Set `tls_version: "1.3"` for maximum security

**Production Startup Validation:**

The coordinator service will **refuse to start** if:
- `JWT_SECRET_KEY` is less than 32 characters
- `COORDINATOR_API_KEY` is less than 32 characters
- Environment is set to `production` without proper secrets

---

## DLL Export Functions (NEW in v2.0.0)

The P2P DLL now exports functions that are called by the NEMO packet hooking patch:

### P2P_RoutePacket

```cpp
extern "C" __declspec(dllexport) int __stdcall P2P_RoutePacket(
    SOCKET socket,
    char* buffer,
    int length,
    int flags,
    int isSend
);
```

**Purpose**: Called by the packet hook to determine if a packet should be routed through P2P or server.

**Returns**:
- `0` = Route to server (call original send/recv)
- `1` = Handled by P2P (skip original send/recv)

**Behavior**:
- Automatically falls back to server if P2P is disabled or unavailable
- Checks packet type to determine routing (movement/chat → P2P, combat/items → server)
- Logs all routing decisions for debugging

### P2P_InjectPacket

```cpp
extern "C" __declspec(dllexport) void __stdcall P2P_InjectPacket(
    const uint8_t* data,
    size_t length
);
```

**Purpose**: Injects received P2P packets into the game's receive buffer.

**Note**: Implementation is client-version specific and requires knowledge of the RO client's packet processing internals.

### P2P_IsActive

```cpp
extern "C" __declspec(dllexport) int __stdcall P2P_IsActive();
```

**Purpose**: Checks if P2P is currently active.

**Returns**:
- `1` = P2P is active
- `0` = P2P is inactive (fallback to server)

### P2P_GetStatus

```cpp
extern "C" __declspec(dllexport) void __stdcall P2P_GetStatus(
    int* is_running,
    int* peer_count,
    int* session_active
);
```

**Purpose**: Retrieves detailed P2P status information.

**Parameters**:
- `is_running`: Whether P2P networking is running
- `peer_count`: Number of connected peers
- `session_active`: Whether in an active P2P session

---

## Zone-Based P2P and Graceful Fallback

### How Zone-Based P2P Works

The P2P system is **zone-aware** and only activates in specific zones. This hybrid approach provides:

- ✅ **Reduced server load** in high-traffic zones (Prontera, Geffen, etc.)
- ✅ **Maintained server control** in critical zones (dungeons, PvP, WoE)
- ✅ **Seamless transitions** between P2P and server modes

### P2P-Enabled Zones

By default, P2P is enabled in these zones:

```json
{
  "zones": {
    "p2p_enabled_zones": [
      "prontera", // Main city
      "geffen", // Magic city
      "payon", // Archer village
      "morocc", // Desert city
      "alberta", // Port city
      "aldebaran", // Clock tower city
      "izlude" // Swordsman city
    ]
  }
}
```

**Customization:** You can add or remove zones based on your server's needs.

### Zone Transition Behavior

**Entering a P2P-Enabled Zone:**

```
Player enters Prontera
  ↓
[info] Zone changed to: prontera
[info] P2P enabled for zone: prontera
  ↓
DLL connects to coordinator signaling server
  ↓
DLL discovers other players in the zone
  ↓
WebRTC peer connections established
  ↓
Player movement/chat packets routed via P2P
```

**Entering a Non-P2P Zone:**

```
Player enters Glast Heim dungeon
  ↓
[info] Zone changed to: glast_heim
[info] P2P disabled for zone: glast_heim
  ↓
DLL disconnects from P2P peers
  ↓
All packets routed via traditional server
```

### Graceful Fallback

The system **automatically falls back** to server communication when:

1. **P2P is disabled in config** (`"enabled": false`)
2. **Coordinator server is unreachable**
3. **WebRTC connection fails** (NAT/firewall issues)
4. **Player is in a non-P2P zone**
5. **Peer connection is lost** (network interruption)

**Fallback is transparent** - players won't notice any difference in gameplay.

### Packet Routing Logic

The DLL intelligently routes packets based on type and zone:

| Packet Type       | P2P Zone | Non-P2P Zone | P2P Failed |
| ----------------- | -------- | ------------ | ---------- |
| Player Movement   | P2P      | Server       | Server     |
| Chat Messages     | P2P      | Server       | Server     |
| Item Pickup       | P2P      | Server       | Server     |
| Skill Usage       | Server   | Server       | Server     |
| Combat Actions    | Server   | Server       | Server     |
| NPC Interactions  | Server   | Server       | Server     |
| Item Transactions | Server   | Server       | Server     |

**Key Point:** Critical game logic (combat, items, NPCs) **always** goes through the server for anti-cheat and validation.

### Monitoring Zone Behavior

**Check current zone status:**

```powershell
# View recent zone changes in log
Select-String -Path p2p_dll.log -Pattern "Zone changed" -Context 0,2
```

**Expected output:**

```
[info] Zone changed to: prontera
[info] P2P enabled for zone: prontera
[info] Connecting to signaling server...

[info] Zone changed to: glast_heim
[info] P2P disabled for zone: glast_heim
[info] Disconnecting from P2P peers...
```

### Configuration Options

**Enable/Disable P2P Globally:**

```json
{
  "p2p": {
    "enabled": true // Set to false to disable P2P entirely
  }
}
```

**Configure Fallback Behavior:**

```json
{
  "zones": {
    "fallback_on_failure": true, // Always fall back to server on error
    "zone_transition_timeout_ms": 5000 // Max time to establish P2P on zone change
  }
}
```

**Add Custom Zones:**

```json
{
  "zones": {
    "p2p_enabled_zones": [
      "prontera",
      "geffen",
      "your_custom_zone" // Add your custom zones here
    ]
  }
}
```

### Testing Zone Behavior

**Test P2P activation:**

1. Enable debug logging:

   ```json
   { "logging": { "level": "debug" } }
   ```

2. Enter a P2P-enabled zone (e.g., Prontera)

3. Check log for peer connections:
   ```
   [debug] Discovered peer: peer_150002
   [debug] WebRTC offer sent to peer_150002
   [debug] Peer connection established: peer_150002
   ```

**Test fallback:**

1. Temporarily disable coordinator in config:

   ```json
   { "coordinator": { "rest_api_url": "http://invalid-url" } }
   ```

2. Enter a P2P zone

3. Check log for fallback:

   ```
   [warn] Failed to connect to coordinator
   [info] Falling back to server communication
   ```

4. Verify gameplay continues normally

---

## Log Files and Monitoring

### Log File Locations

The P2P DLL creates log files in the **same directory as the RO client executable**:

```
RagnarokOnline/
├── Ragnarok_P2P.exe
├── p2p_dll.log          ← Current log file
├── p2p_dll.1.log        ← Rotated log (previous)
├── p2p_dll.2.log        ← Rotated log (older)
├── p2p_dll.3.log
├── p2p_dll.4.log
└── p2p_dll.5.log        ← Oldest log (max 5 files)
```

**Log Rotation:**

- Maximum file size: 10 MB (configurable)
- Maximum files: 5 (configurable)
- Oldest logs are automatically deleted

### Log Levels

Configure logging verbosity in `p2p_config.json`:

```json
{
  "logging": {
    "level": "info" // Options: "debug", "info", "warn", "error"
  }
}
```

| Level   | Use Case                         | Output Volume |
| ------- | -------------------------------- | ------------- |
| `debug` | Development, troubleshooting     | Very High     |
| `info`  | Production, normal operation     | Medium        |
| `warn`  | Production, errors only          | Low           |
| `error` | Production, critical errors only | Very Low      |

**Recommendation:** Use `"info"` for production, `"debug"` for troubleshooting.

### What to Look For in Logs

**✅ Successful DLL Loading:**

```
[info] === P2P Network DLL Loaded ===
[info] DLL Path: C:\RagnarokOnline\p2p_network.dll
[info] Config Path: C:\RagnarokOnline\p2p_config.json
[info] P2P networking is ENABLED
[info] === P2P Network DLL Initialization Complete ===
```

**✅ Successful Coordinator Connection:**

```
[info] Starting P2P networking...
[info] Player ID: 150001
[info] User ID: 2000001
[info] Authenticating with coordinator...
[info] Authentication successful
[info] NetworkManager started
```

**✅ Zone Change and P2P Activation:**

```
[info] Zone changed to: prontera
[info] P2P enabled for zone: prontera
[info] Connecting to signaling server...
[info] Connected to: wss://coordinator.example.com/api/v1/signaling/ws
```

**✅ Peer Connection Established:**

```
[info] Discovered peer: peer_150002
[debug] WebRTC offer sent to peer_150002
[debug] WebRTC answer received from peer_150002
[debug] ICE candidate added
[info] Peer connection established: peer_150002
```

**❌ Common Error Messages:**

```
[error] Failed to load configuration from: p2p_config.json
→ Solution: Check that p2p_config.json exists and is valid JSON

[error] Failed to connect to coordinator
→ Solution: Check coordinator URL, network connectivity, firewall

[warn] P2P is disabled in configuration
→ Solution: Set "enabled": true in p2p_config.json

[error] Authentication failed: Invalid API key
→ Solution: Check API key in p2p_config.json

[error] ICE connection failed
→ Solution: Check STUN/TURN servers, firewall allows UDP

[error] The application was unable to start correctly (0xc000007b)
→ Solution: Install Visual C++ Redistributable 2022
```

### Monitoring Commands

**View recent log entries:**

```powershell
# Last 20 lines
Get-Content p2p_dll.log -Tail 20

# Last 50 lines
Get-Content p2p_dll.log -Tail 50

# Follow log in real-time
Get-Content p2p_dll.log -Wait -Tail 10
```

**Search for errors:**

```powershell
# Find all errors
Select-String -Path p2p_dll.log -Pattern "error|failed|exception" -CaseSensitive:$false

# Find connection issues
Select-String -Path p2p_dll.log -Pattern "connection|timeout|failed to connect"

# Find authentication issues
Select-String -Path p2p_dll.log -Pattern "auth|api key|jwt"
```

**Check P2P activity:**

```powershell
# Find peer connections
Select-String -Path p2p_dll.log -Pattern "peer.*established|discovered peer"

# Find zone changes
Select-String -Path p2p_dll.log -Pattern "Zone changed"

# Find packet routing
Select-String -Path p2p_dll.log -Pattern "Sending packet.*via P2P"
```

**Analyze log statistics:**

```powershell
# Count errors
(Select-String -Path p2p_dll.log -Pattern "\[error\]").Count

# Count warnings
(Select-String -Path p2p_dll.log -Pattern "\[warn\]").Count

# Count peer connections
(Select-String -Path p2p_dll.log -Pattern "Peer connection established").Count
```

### Log File Management

**Clear old logs:**

```powershell
# Delete all rotated logs (keep current)
Remove-Item p2p_dll.*.log

# Delete all logs
Remove-Item p2p_dll*.log
```

**Archive logs:**

```powershell
# Create archive of logs
$date = Get-Date -Format "yyyy-MM-dd"
Compress-Archive -Path "p2p_dll*.log" -DestinationPath "logs_archive_$date.zip"
```

**Configure log retention:**

Edit `p2p_config.json`:

```json
{
  "logging": {
    "file": "p2p_dll.log",
    "max_file_size_mb": 10, // Max size before rotation
    "max_files": 5, // Max number of rotated files
    "console_output": false, // Set true to also output to console
    "async_logging": true // Async logging for better performance
  }
}
```

### Troubleshooting with Logs

**Problem: Client crashes on startup**

```powershell
# Check if log file was created
Test-Path p2p_dll.log

# If no log file exists:
# → DLL not loading (check NEMO patch, DLL dependencies)

# If log file exists, check last entries:
Get-Content p2p_dll.log -Tail 10
```

**Problem: P2P not working**

```powershell
# Check if P2P is enabled
Select-String -Path p2p_dll.log -Pattern "P2P networking is (ENABLED|DISABLED)"

# Check coordinator connection
Select-String -Path p2p_dll.log -Pattern "coordinator|authentication"

# Check zone activation
Select-String -Path p2p_dll.log -Pattern "Zone changed|P2P enabled for zone"
```

**Problem: High latency or packet loss**

```powershell
# Enable debug logging first
# Edit p2p_config.json: "level": "debug"

# Check packet routing
Select-String -Path p2p_dll.log -Pattern "Sending packet|Packet sent|Packet failed"

# Check peer connection quality
Select-String -Path p2p_dll.log -Pattern "latency|packet loss|connection quality"
```

---

## Coordinator Server Deployment

The coordinator server handles peer discovery, signaling, and session management.

### Server Requirements

**Minimum Specifications:**

- **CPU:** 2 cores
- **RAM:** 2 GB
- **Network:** 100 Mbps
- **OS:** Linux (Ubuntu 20.04+) or Windows Server 2019+
- **SSL Certificate:** Valid SSL certificate for HTTPS/WSS

**Recommended Specifications (1000+ concurrent users):**

- **CPU:** 4+ cores
- **RAM:** 8+ GB
- **Network:** 1 Gbps
- **Load Balancer:** For horizontal scaling

### Server Components

#### 1. REST API Server

**Endpoints:**

- `POST /api/v1/auth/login` - Authenticate user
- `POST /api/v1/sessions/join` - Join zone session
- `POST /api/v1/sessions/leave` - Leave zone session
- `GET /api/v1/sessions/{zone_id}` - Get zone session info
- `GET /api/v1/peers/{zone_id}` - Get peers in zone

**Technology Stack:**

- **Node.js + Express** (recommended)
- **Python + FastAPI** (alternative)
- **Go + Gin** (high performance)

#### 2. WebSocket Signaling Server

**Purpose:** Real-time SDP/ICE exchange between peers

**Messages:**

- `offer` - WebRTC offer from initiating peer
- `answer` - WebRTC answer from receiving peer
- `ice-candidate` - ICE candidate from either peer

**Technology Stack:**

- **Node.js + Socket.IO** (recommended)
- **Python + websockets**
- **Go + gorilla/websocket**

### Example Deployment (Node.js)

**1. Install Dependencies:**

```bash
npm install express socket.io jsonwebtoken bcrypt
```

**2. Basic Server Implementation:**

```javascript
// server.js
const express = require("express");
const https = require("https");
const socketIO = require("socket.io");
const jwt = require("jsonwebtoken");
const fs = require("fs");

const app = express();
app.use(express.json());

// SSL certificates
const server = https.createServer(
  {
    key: fs.readFileSync("/path/to/privkey.pem"),
    cert: fs.readFileSync("/path/to/fullchain.pem"),
  },
  app
);

const io = socketIO(server, {
  cors: { origin: "*" },
});

// In-memory session storage (use Redis in production)
const sessions = new Map();

// REST API: Join session
app.post("/api/v1/sessions/join", (req, res) => {
  const { zone_id, peer_id, user_id } = req.body;

  // Verify JWT token
  const token = req.headers.authorization?.split(" ")[1];
  if (!token) return res.status(401).json({ error: "Unauthorized" });

  try {
    jwt.verify(token, process.env.JWT_SECRET);
  } catch (err) {
    return res.status(401).json({ error: "Invalid token" });
  }

  // Add peer to session
  if (!sessions.has(zone_id)) {
    sessions.set(zone_id, new Set());
  }
  sessions.get(zone_id).add(peer_id);

  // Return list of peers in zone
  const peers = Array.from(sessions.get(zone_id)).filter((p) => p !== peer_id);
  res.json({ peers });
});

// WebSocket: Signaling
io.on("connection", (socket) => {
  console.log("Client connected:", socket.id);

  socket.on("offer", (data) => {
    const { target_peer_id, sdp } = data;
    io.to(target_peer_id).emit("offer", {
      from_peer_id: socket.id,
      sdp,
    });
  });

  socket.on("answer", (data) => {
    const { target_peer_id, sdp } = data;
    io.to(target_peer_id).emit("answer", {
      from_peer_id: socket.id,
      sdp,
    });
  });

  socket.on("ice-candidate", (data) => {
    const { target_peer_id, candidate } = data;
    io.to(target_peer_id).emit("ice-candidate", {
      from_peer_id: socket.id,
      candidate,
    });
  });

  socket.on("disconnect", () => {
    console.log("Client disconnected:", socket.id);
    // Remove peer from all sessions
    sessions.forEach((peers, zone_id) => {
      peers.delete(socket.id);
    });
  });
});

server.listen(443, () => {
  console.log("Coordinator server running on port 443");
});
```

**3. Run Server:**

```bash
export JWT_SECRET="your-secret-key"
node server.js
```

**4. Production Deployment:**

Use **PM2** for process management:

```bash
npm install -g pm2
pm2 start server.js --name coordinator
pm2 startup
pm2 save
```

### STUN/TURN Server Setup

**Option 1: Use Public STUN Servers**

Free public STUN servers (limited reliability):

- `stun:stun.l.google.com:19302`
- `stun:stun1.l.google.com:19302`
- `stun:stun.stunprotocol.org:3478`

**Option 2: Deploy Your Own TURN Server (Recommended)**

Use **coturn** for STUN/TURN:

```bash
# Install coturn
sudo apt-get update
sudo apt-get install coturn

# Configure coturn
sudo nano /etc/turnserver.conf
```

**coturn Configuration:**

```conf
# /etc/turnserver.conf
listening-port=3478
tls-listening-port=5349
listening-ip=0.0.0.0
relay-ip=YOUR_SERVER_IP
external-ip=YOUR_PUBLIC_IP

realm=your-domain.com
server-name=your-domain.com

lt-cred-mech
user=username:password

cert=/etc/letsencrypt/live/your-domain.com/fullchain.pem
pkey=/etc/letsencrypt/live/your-domain.com/privkey.pem

no-stdout-log
log-file=/var/log/turnserver.log
```

**Start coturn:**

```bash
sudo systemctl enable coturn
sudo systemctl start coturn
```

**Firewall Rules:**

```bash
# Allow STUN/TURN ports
sudo ufw allow 3478/tcp
sudo ufw allow 3478/udp
sudo ufw allow 5349/tcp
sudo ufw allow 49152:65535/udp  # TURN relay ports
```

---

## Security Considerations

### 1. Transport Security

**✅ ALWAYS use HTTPS/WSS in production**

```json
{
  "coordinator": {
    "rest_api_url": "https://your-server.com/api/v1", // ✅ HTTPS
    "websocket_url": "wss://your-server.com/api/v1/signaling/ws" // ✅ WSS
  }
}
```

**❌ NEVER use HTTP/WS in production:**

```json
{
  "coordinator": {
    "rest_api_url": "http://your-server.com/api/v1", // ❌ Insecure!
    "websocket_url": "ws://your-server.com/api/v1/signaling/ws" // ❌ Insecure!
  }
}
```

### 2. Authentication

**API Key Management:**

- Generate unique API keys per deployment
- Store API keys securely (environment variables, key vault)
- Rotate API keys regularly (every 90 days)
- Never commit API keys to version control

**JWT Token Security:**

- Use strong secret keys (256-bit minimum)
- Set appropriate expiration times (1-24 hours)
- Implement token refresh mechanism
- Validate tokens on every request

### 3. Packet Encryption

**Enable encryption in production:**

```json
{
  "security": {
    "enable_encryption": true, // ✅ Always enable
    "enable_authentication": true
  }
}
```

**Encryption Details:**

- Algorithm: AES-256-GCM
- Key Exchange: ECDH (via WebRTC DTLS)
- Perfect Forward Secrecy: Yes

### 4. Rate Limiting

**Implement rate limiting on coordinator server:**

```javascript
const rateLimit = require("express-rate-limit");

const limiter = rateLimit({
  windowMs: 15 * 60 * 1000, // 15 minutes
  max: 100, // limit each IP to 100 requests per windowMs
});

app.use("/api/", limiter);
```

### 5. Input Validation

**Validate all inputs:**

- Sanitize zone IDs, peer IDs, user IDs
- Validate packet sizes (max 64KB)
- Check SDP/ICE candidate formats
- Prevent injection attacks

### 6. DDoS Protection

**Mitigation Strategies:**

- Use CDN/DDoS protection (Cloudflare, AWS Shield)
- Implement connection limits per IP
- Use SYN cookies for TCP
- Monitor traffic patterns

### 7. Firewall Configuration

**Client-Side (End Users):**

```powershell
# Allow UDP for WebRTC
New-NetFirewallRule -DisplayName "P2P WebRTC" -Direction Outbound -Protocol UDP -Action Allow
```

**Server-Side:**

```bash
# Coordinator server
sudo ufw allow 443/tcp  # HTTPS/WSS

# TURN server
sudo ufw allow 3478/tcp
sudo ufw allow 3478/udp
sudo ufw allow 5349/tcp
sudo ufw allow 49152:65535/udp
```

### 8. Logging and Monitoring

**Enable comprehensive logging:**

```json
{
  "logging": {
    "level": "INFO", // Use INFO in production
    "file": "logs/p2p_network.log",
    "max_file_size_mb": 10,
    "max_files": 5
  }
}
```

**Monitor for:**

- Failed authentication attempts
- Unusual traffic patterns
- Connection failures
- High latency/packet loss

---

## Testing Deployment

### Pre-Deployment Checklist

- [ ] DLL built in Release mode
- [ ] All dependencies copied to client directory
- [ ] Configuration file created and validated
- [ ] Coordinator server running and accessible
- [ ] SSL certificates valid and not expired
- [ ] STUN/TURN servers accessible
- [ ] Firewall rules configured
- [ ] Visual C++ Redistributable installed on test machine

### Test Procedure

**1. Test DLL Loading:**

```powershell
# Launch patched client
cd C:\RagnarokOnline
.\Ragnarok_P2P.exe
```

**Expected:** Client launches without errors

**2. Check Log File:**

```powershell
# View log file
Get-Content logs\p2p_network.log -Tail 20
```

**Expected Output:**

```
[2025-11-08 10:00:00.123] [info] P2P DLL loaded successfully
[2025-11-08 10:00:00.234] [info] Configuration loaded from config/p2p_config.json
[2025-11-08 10:00:00.345] [info] P2P networking initialized
```

**3. Test Coordinator Connection:**

```powershell
# Test REST API
curl https://your-server.com/api/v1/health

# Test WebSocket
# Use browser console or wscat
wscat -c wss://your-server.com/api/v1/signaling/ws
```

**Expected:** 200 OK response, WebSocket connection established

**4. Test P2P Connection:**

- Launch 2 clients on different machines
- Log in with different accounts
- Enter same zone (e.g., Prontera)
- Check logs for peer connection messages

**Expected Log Output:**

```
[2025-11-08 10:05:00.123] [info] Joined zone: prontera
[2025-11-08 10:05:00.234] [info] Discovered peer: peer_150002
[2025-11-08 10:05:00.345] [info] WebRTC offer sent to peer_150002
[2025-11-08 10:05:00.456] [info] WebRTC answer received from peer_150002
[2025-11-08 10:05:00.567] [info] ICE candidate added
[2025-11-08 10:05:00.678] [info] Peer connection established: peer_150002
```

**5. Test Packet Routing:**

- Move character in-game
- Check logs for P2P packet transmission

**Expected:**

```
[2025-11-08 10:06:00.123] [debug] Sending packet 0x0088 via P2P to peer_150002
[2025-11-08 10:06:00.234] [debug] Packet sent successfully
```

### Common Issues

See [Troubleshooting](#troubleshooting) section below.

---

## Troubleshooting

### Issue 1: DLL Not Loading

**Symptoms:**

- Client crashes on startup
- No log file created
- Error: "The application was unable to start correctly (0xc000007b)"

**Solutions:**

1. **Install Visual C++ Redistributable:**

   ```powershell
   # Download and install
   Invoke-WebRequest -Uri "https://aka.ms/vs/17/release/vc_redist.x64.exe" -OutFile "vc_redist.x64.exe"
   .\vc_redist.x64.exe /install /quiet /norestart
   ```

2. **Check DLL Dependencies:**

   ```powershell
   # Use Dependencies.exe to check missing DLLs
   # Download from: https://github.com/lucasg/Dependencies
   .\Dependencies.exe -imports p2p_network.dll
   ```

3. **Verify DLL Architecture:**

   - Ensure DLL is x64 (not x86)
   - Ensure client is x64 (not x86)

4. **Check NEMO Patch:**
   - Verify "Load P2P Network DLL" patch was applied
   - Re-patch client if necessary

---

### Issue 2: Cannot Connect to Coordinator

**Symptoms:**

- Log shows: "Failed to connect to coordinator"
- Log shows: "Connection timeout"
- P2P status shows: `"p2p_active": false`

**Solutions:**

1. **Verify Coordinator URL:**

   ```json
   {
     "coordinator": {
       "rest_api_url": "https://your-server.com/api/v1", // Check URL
       "websocket_url": "wss://your-server.com/api/v1/signaling/ws"
     }
   }
   ```

2. **Test Coordinator Connectivity:**

   ```powershell
   # Test HTTPS
   curl https://your-server.com/api/v1/health

   # Test DNS resolution
   nslookup your-server.com

   # Test port connectivity
   Test-NetConnection -ComputerName your-server.com -Port 443
   ```

3. **Check Firewall:**

   ```powershell
   # Allow outbound HTTPS
   New-NetFirewallRule -DisplayName "P2P HTTPS" -Direction Outbound -Protocol TCP -RemotePort 443 -Action Allow
   ```

4. **Verify SSL Certificate:**
   - Ensure coordinator has valid SSL certificate
   - Check certificate expiration date
   - Disable `certificate_validation` temporarily for testing (NOT in production!)

---

### Issue 3: WebRTC Connection Fails

**Symptoms:**

- Log shows: "ICE connection failed"
- Log shows: "Peer connection timeout"
- Peers discovered but not connected

**Solutions:**

1. **Check STUN/TURN Servers:**

   ```json
   {
     "webrtc": {
       "stun_servers": [
         "stun:stun.l.google.com:19302" // Verify accessible
       ]
     }
   }
   ```

2. **Test STUN Server:**

   ```powershell
   # Use online STUN tester: https://webrtc.github.io/samples/src/content/peerconnection/trickle-ice/
   ```

3. **Allow UDP Traffic:**

   ```powershell
   # Allow outbound UDP for WebRTC
   New-NetFirewallRule -DisplayName "P2P WebRTC UDP" -Direction Outbound -Protocol UDP -Action Allow
   ```

4. **Use TURN Server:**

   - If behind strict NAT/firewall, TURN server is required
   - Add TURN server to configuration:

   ```json
   {
     "webrtc": {
       "turn_servers": ["turn:username:password@your-turn-server.com:3478"]
     }
   }
   ```

5. **Check NAT Type:**
   - Symmetric NAT may prevent P2P connections
   - Use TURN server as relay

---

### Issue 4: High Latency or Packet Loss

**Symptoms:**

- Log shows high latency values (>200ms)
- Log shows packet loss (>5%)
- Gameplay feels laggy

**Solutions:**

1. **Check Network Quality:**

   ```powershell
   # Ping coordinator server
   ping your-server.com

   # Traceroute
   tracert your-server.com
   ```

2. **Reduce Max Peers:**

   ```json
   {
     "p2p": {
       "max_peers": 20 // Reduce from 50
     }
   }
   ```

3. **Enable Congestion Control:**

   ```json
   {
     "p2p": {
       "enable_congestion_control": true
     }
   }
   ```

4. **Adjust Bandwidth Limits:**
   ```json
   {
     "p2p": {
       "max_bandwidth_mbps": 5, // Reduce if limited bandwidth
       "target_bitrate_kbps": 500
     }
   }
   ```

---

### Issue 5: Authentication Failures

**Symptoms:**

- Log shows: "Authentication failed"
- Log shows: "Invalid API key"
- HTTP 401 Unauthorized errors

**Solutions:**

1. **Verify API Key:**

   ```json
   {
     "security": {
       "api_key": "your-correct-api-key-here" // Check with server admin
     }
   }
   ```

2. **Check JWT Token:**

   - Ensure JWT token is not expired
   - Implement token refresh mechanism
   - Verify JWT secret matches server

3. **Test Authentication:**
   ```powershell
   # Test API key
   curl -H "X-API-Key: your-api-key" https://your-server.com/api/v1/auth/login
   ```

---

### Issue 6: Encryption Errors

**Symptoms:**

- Log shows: "Decryption failed"
- Log shows: "Invalid ciphertext"
- Packets not being received

**Solutions:**

1. **Verify Encryption Settings Match:**

   - All clients must have same `enable_encryption` setting
   - Coordinator must support encryption

2. **Check OpenSSL DLLs:**

   ```powershell
   # Verify OpenSSL DLLs are present
   Test-Path "libssl-3-x64.dll"
   Test-Path "libcrypto-3-x64.dll"
   ```

3. **Temporarily Disable Encryption (Testing Only):**
   ```json
   {
     "security": {
       "enable_encryption": false // ⚠️ TESTING ONLY!
     }
   }
   ```

---

### Issue 7: Memory Leaks or Crashes

**Symptoms:**

- Client crashes after extended play
- Memory usage increases over time
- Log shows: "Access violation"

**Solutions:**

1. **Update to Latest DLL Version:**

   - Rebuild DLL with latest code
   - Check for known issues in repository

2. **Enable Debug Logging:**

   ```json
   {
     "logging": {
       "level": "DEBUG" // Capture more details
     }
   }
   ```

3. **Check for Resource Leaks:**

   - Monitor peer connection count
   - Ensure old connections are closed
   - Check log for "Connection closed" messages

4. **Report Bug:**
   - Collect crash dump
   - Collect log file
   - Report to development team with reproduction steps

---

### Issue 8: Configuration Not Loading

**Symptoms:**

- Log shows: "Using default configuration"
- Log shows: "Failed to load config file"
- Settings not applied

**Solutions:**

1. **Verify Config File Path:**

   ```powershell
   # Check file exists
   Test-Path "config\p2p_config.json"

   # Check file permissions
   Get-Acl "config\p2p_config.json"
   ```

2. **Validate JSON Syntax:**

   ```powershell
   # Use online JSON validator: https://jsonlint.com/
   # Or use PowerShell
   Get-Content "config\p2p_config.json" | ConvertFrom-Json
   ```

3. **Check File Encoding:**

   - Ensure UTF-8 encoding (no BOM)
   - No special characters in file path

4. **Use Absolute Path:**
   ```cpp
   // In P2P_Initialize call
   P2P_Initialize("C:\\RagnarokOnline\\config\\p2p_config.json");
   ```

---

### Diagnostic Commands

**View Recent Logs:**

```powershell
Get-Content logs\p2p_network.log -Tail 50
```

**Search Logs for Errors:**

```powershell
Select-String -Path logs\p2p_network.log -Pattern "error|failed|exception" -CaseSensitive:$false
```

**Check DLL Version:**

```powershell
# View DLL properties
Get-Item p2p_network.dll | Select-Object VersionInfo
```

**Monitor Network Connections:**

```powershell
# View active connections
Get-NetTCPConnection | Where-Object {$_.State -eq "Established"}
```

---

## Additional Resources

- **[BUILD_GUIDE.md](BUILD_GUIDE.md)** - Build instructions and troubleshooting
- **[API_REFERENCE.md](API_REFERENCE.md)** - Complete API documentation
- **[WEBRTC_GUIDE.md](WEBRTC_GUIDE.md)** - WebRTC implementation details
- **[README.md](README.md)** - Project overview

### Support

- **GitHub Issues:** https://github.com/your-repo/WARP-p2p-client/issues
- **Discord:** [Your Discord Server]
- **Email:** support@your-domain.com

---

## Deployment Checklist

Use this checklist to ensure successful deployment:

### Pre-Deployment

- [ ] DLL built in Release mode with optimizations
- [ ] All dependencies identified and collected
- [ ] Configuration file created and validated
- [ ] Coordinator server deployed and tested
- [ ] STUN/TURN servers configured
- [ ] SSL certificates obtained and installed
- [ ] Firewall rules configured on server
- [ ] Rate limiting implemented on coordinator
- [ ] Logging and monitoring configured

### Client Deployment

- [ ] NEMO patch script tested
- [ ] RO client patched successfully
- [ ] DLL and dependencies copied to client directory
- [ ] Configuration file deployed
- [ ] Visual C++ Redistributable installer included
- [ ] Installation instructions written for end users
- [ ] Test deployment on clean machine

### Post-Deployment

- [ ] Monitor logs for errors
- [ ] Monitor server load and performance
- [ ] Test P2P connections between multiple clients
- [ ] Verify packet routing working correctly
- [ ] Check for memory leaks or crashes
- [ ] Collect user feedback
- [ ] Document any issues encountered

---

**Deployment Guide Version:** 1.0.0
**Last Updated:** November 8, 2025
**Maintained by:** rAthena AI World Development Team
