# P2P Network DLL - Quick Deployment Guide

**Status:** ✅ P2P DLL is already built and deployed!  
**Platform:** Windows 10/11 (x64)  
**Last Updated:** 2025-06-04

---

## 🎯 Current Status

✅ **P2P DLL Built** - `p2p_network.dll` (568 KB) in `d:\RO\client\`  
✅ **All Dependencies Deployed** - 7 DLLs in `d:\RO\client\`  
✅ **WARP Patcher Available** - `d:\RO\patcher\WARP-p2p-client\win32\WARP.exe`  
✅ **RO Client Ready** - Multiple executables in `d:\RO\client\`  

**What's left:** Copy configuration file and test!

---

## 📋 Quick Start (3 Steps)

### Step 1: Copy Configuration File

```powershell
# Copy P2P configuration to client directory
Copy-Item "d:\RO\patcher\WARP-p2p-client\P2P-DLL\config\p2p_config.json" -Destination "d:\RO\client\" -Force
```

### Step 2: Verify DLLs Are Present

```powershell
# Check that all DLLs are in client directory
cd d:\RO\client
Get-ChildItem p2p_network.dll, libcrypto-3-x64.dll, libssl-3-x64.dll, spdlog.dll, fmt.dll, brotlicommon.dll, brotlidec.dll
```

**Expected output:**
```
p2p_network.dll       568,832 bytes
libcrypto-3-x64.dll   5,327,872 bytes
libssl-3-x64.dll      871,424 bytes
spdlog.dll            285,696 bytes
fmt.dll               120,832 bytes
brotlicommon.dll      137,728 bytes
brotlidec.dll         52,224 bytes
```

### Step 3: Run Client

```powershell
cd d:\RO\client
.\2025-06-04_Ragexe.exe
```

**Check logs:**
```powershell
# View P2P DLL log
Get-Content p2p_dll.log -Tail 20
```

---

## 🔧 Deployment Methods

### Method 1: Direct Execution (Simplest)

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

Edit `d:\RO\client\p2p_config.json`:

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

**Key settings:**
- `coordinator.rest_api_url` - Coordinator server URL
- `coordinator.websocket_url` - WebSocket signaling URL
- `webrtc.stun_servers` - STUN servers for NAT traversal
- `p2p.enabled` - Enable/disable P2P (set to `false` to disable)

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
# Check if coordinator is running
Invoke-WebRequest -Uri "http://localhost:8001/api/v1/health"
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

