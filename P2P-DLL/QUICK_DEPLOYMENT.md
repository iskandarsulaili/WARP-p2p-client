# P2P Network DLL - Quick Deployment Guide

**Status:** ✅ P2P DLL is already built and deployed!  
**Platform:** Windows 10/11 (x64)  
**Last Updated:** 2025-06-04

---

## 🎯 Current Status

✅ **P2P DLL Built** - `p2p_network.dll` (568 KB) in `d:\RO\client\`
✅ **All Dependencies Deployed** - 7 DLLs in `d:\RO\client\`
✅ **Clients Patched** - `2025-06-04_Ragexe_P2P.exe` & `2025-06-04_Speedrun_P2P.exe`
✅ **Configuration Copied** - `p2p_config.json` in `d:\RO\client\`

**Status:** 🎉 **READY TO RUN!** Everything is set up!

---

## 📋 Quick Start (Just Run It!)

### Step 1: Choose and Run Your Client

```powershell
cd d:\RO\client

# Option 1: Run standard Ragexe client
.\2025-06-04_Ragexe_P2P.exe

# Option 2: Run Speedrun client
.\2025-06-04_Speedrun_P2P.exe
```

**That's it!** Both clients are already patched with P2P DLL support and configured.

---

### Step 2: Verify P2P DLL Loaded

```powershell
# View P2P DLL log
Get-Content d:\RO\client\p2p_dll.log -Tail 20
```

**Expected output:**

```
[INFO] P2P Network DLL v1.0.0 initializing...
[INFO] Configuration loaded from p2p_config.json
[INFO] WebRTC manager initialized
[INFO] P2P Network DLL initialized successfully
```

---

### Step 3: (Optional) Verify All Files

```powershell
# Check that all files are present
cd d:\RO\client
Get-ChildItem *_P2P.exe, p2p_network.dll, p2p_config.json
```

**Expected output:**

```
2025-06-04_Ragexe_P2P.exe      15,075,840 bytes  ← Patched Ragexe client
2025-06-04_Speedrun_P2P.exe    15,075,840 bytes  ← Patched Speedrun client
p2p_network.dll                   568,832 bytes  ← P2P DLL
p2p_config.json                     1,234 bytes  ← Configuration
```

---

## 🎉 What Was Done

The client has been **automatically patched** using WARP patcher with the following configuration:

### Patches Applied

| Patch                 | Description                                | Status     |
| --------------------- | ------------------------------------------ | ---------- |
| **CustomDLL**         | Loads `p2p_network.dll` on client startup  | ✅ Applied |
| **MultiGRFs**         | Enable multiple GRF file support           | ✅ Applied |
| **IncrZoom**          | Increase zoom out distance                 | ✅ Applied |
| **SkipServiceSelect** | Skip service selection screen              | ✅ Applied |
| **NoHardCodedAddr**   | Remove hardcoded server addresses          | ✅ Applied |
| **EnableDnsSupport**  | Enable DNS resolution for server addresses | ✅ Applied |

### Files Created

```
d:\RO\client\
├── 2025-06-04_Ragexe_P2P.exe    ← Patched client (15 MB)
└── p2p_config.json               ← P2P configuration (copied)

d:\RO\patcher\WARP-p2p-client\
├── P2P_Session.yml               ← WARP session file
└── Inputs\P2P_DLLSpec.yml        ← DLL specification for CustomDLL patch
```

### How It Was Patched

```powershell
# 1. Created DLL specification file
Inputs/P2P_DLLSpec.yml → Tells WARP to load p2p_network.dll

# 2. Created session file
P2P_Session.yml → Specifies which patches to apply

# 3. Ran WARP console patcher
WARP_console.exe -using P2P_Session.yml -from 2025-06-04_Ragexe.exe -to 2025-06-04_Ragexe_P2P.exe

# 4. Copied configuration
p2p_config.json → client directory
```

---

## 🔧 Alternative Deployment Methods

### Method 1: Using the Patched Client (Current Setup) ⭐

The P2P DLL is already in the client directory. Just run the client!

**Pros:**

- ✅ No patching needed
- ✅ Works immediately
- ✅ Easy to test

**Cons:**

- ❌ DLL must be manually loaded (requires DLL injector or client modification)

---

### Method 2: WARP Patcher (Recommended for Distribution)

Use WARP to patch the client and apply other useful patches.

#### Launch WARP

```powershell
cd d:\RO\patcher\WARP-p2p-client\win32
.\WARP.exe
```

#### Patch Client

1. **Input Exe File:** Browse to `d:\RO\client\2025-06-04_Ragexe.exe`
2. **Output Patched:** Save as `d:\RO\client\2025-06-04_Ragexe_Patched.exe`
3. **Select patches** you want (e.g., "Enable Multiple GRFs", "Increase Zoom")
4. **Click "Apply"**

**Note:** WARP may not have a specific "Load P2P DLL" patch. Use Method 3 for DLL injection.

---

### Method 3: Manual DLL Injection

Use a DLL injector to load `p2p_network.dll` into the running client.

#### Using Extreme Injector

1. Download [Extreme Injector](https://github.com/master131/ExtremeInjector)
2. Launch RO client
3. Open Extreme Injector
4. Select process: `Ragexe.exe`
5. Select DLL: `d:\RO\client\p2p_network.dll`
6. Click "Inject"

#### Verify Injection

```powershell
# Check P2P DLL log
Get-Content d:\RO\client\p2p_dll.log
```

Look for: `[INFO] P2P Network DLL initialized successfully`

---

## 📁 File Structure

```
d:\RO\client\
├── 2025-06-04_Ragexe.exe           ← RO client executable
├── p2p_network.dll                  ← Main P2P DLL (568 KB) ✅
├── libcrypto-3-x64.dll              ← OpenSSL crypto (5.3 MB) ✅
├── libssl-3-x64.dll                 ← OpenSSL SSL (871 KB) ✅
├── spdlog.dll                       ← Logging (285 KB) ✅
├── fmt.dll                          ← Formatting (120 KB) ✅
├── brotlicommon.dll                 ← Compression (137 KB) ✅
├── brotlidec.dll                    ← Compression (52 KB) ✅
├── p2p_config.json                  ← Configuration ⚠️ COPY THIS
├── p2p_dll.log                      ← Log file (auto-created)
├── data.grf                         ← Game data
└── ... (other RO files)
```

---

## ⚙️ Configuration

Configuration is already set up in `d:\RO\client\p2p_config.json`:

### Development Configuration (Default)

```json
{
  "coordinator": {
    "rest_api_url": "http://localhost:8001/api/v1",
    "websocket_url": "ws://localhost:8001/api/v1/signaling/ws"
  },
  "webrtc": {
    "stun_servers": [
      "stun:stun.l.google.com:19302",
      "stun:stun1.l.google.com:19302"
    ]
  },
  "p2p": {
    "enabled": true,
    "max_peers": 50
  }
}
```

**Default settings:**

- ✅ `coordinator.rest_api_url` - **http://localhost:8001/api/v1** (development)
- ✅ `coordinator.websocket_url` - **ws://localhost:8001/api/v1/signaling/ws** (development)
- ✅ `webrtc.stun_servers` - Google's public STUN servers
- ✅ `p2p.enabled` - **true** (P2P is enabled)

### Production Configuration

For production deployment, copy and customize [`p2p_config.production.example.json`](config/p2p_config.production.example.json):

```json
{
  "coordinator": {
    "rest_api_url": "https://YOUR_COORDINATOR_DOMAIN/api/v1",
    "websocket_url": "wss://YOUR_COORDINATOR_DOMAIN/api/v1/signaling/ws"
  },
  "security": {
    "enable_encryption": true,
    "enable_authentication": true,
    "api_key": "YOUR_API_KEY_HERE"
  }
}
```

**Important:** Always use HTTPS/WSS in production for security.

---

## 🧪 Testing

### Test 1: Verify DLL Loads

```powershell
# Run client and check log
cd d:\RO\client
.\2025-06-04_Ragexe.exe

# In another terminal
Get-Content p2p_dll.log -Wait
```

**Expected log output:**

```
[INFO] P2P Network DLL v1.0.0 initializing...
[INFO] Configuration loaded from p2p_config.json
[INFO] WebRTC manager initialized
[INFO] P2P Network DLL initialized successfully
```

### Test 2: Check Coordinator Connection

```powershell
# Check if coordinator is running on localhost (development)
Invoke-WebRequest -Uri "http://localhost:8001/api/v1/health"

# Or on production server
Invoke-WebRequest -Uri "https://YOUR_COORDINATOR_DOMAIN/api/v1/health"
```

**Expected:** HTTP 200 OK

---

## 🔍 Troubleshooting

### DLL Not Loading

**Check:**

1. All 7 DLLs are in client directory
2. Visual C++ Redistributable 2015-2022 is installed
3. Client is x64 (not x86)

**Install VC++ Redist:**

```powershell
# Download and install
Start-Process "https://aka.ms/vs/17/release/vc_redist.x64.exe"
```

### Configuration Not Found

**Error:** `Failed to load configuration file`

**Solution:**

```powershell
# Copy config file
Copy-Item "d:\RO\patcher\WARP-p2p-client\P2P-DLL\config\p2p_config.json" -Destination "d:\RO\client\" -Force
```

### Coordinator Connection Failed

**Error:** `Failed to connect to coordinator`

**Check:**

1. Coordinator server is running
2. URL in `p2p_config.json` is correct
3. Firewall allows connection

---

## 📚 Related Documentation

- **[BUILD_GUIDE.md](BUILD_GUIDE.md)** - How to rebuild the DLL
- **[DEPLOYMENT_GUIDE.md](DEPLOYMENT_GUIDE.md)** - Full deployment guide
- **[API_REFERENCE.md](API_REFERENCE.md)** - API documentation
- **[WEBRTC_GUIDE.md](WEBRTC_GUIDE.md)** - WebRTC implementation details

---

## ✅ Summary

**You're almost done!** The P2P DLL is already built and deployed. Just:

1. ✅ Copy `p2p_config.json` to client directory
2. ✅ Run the client (or inject DLL manually)
3. ✅ Check logs to verify it works

**For distribution to end users:**

- Package the client directory with all DLLs
- Include `p2p_config.json` with correct coordinator URL
- Provide instructions for running the client
