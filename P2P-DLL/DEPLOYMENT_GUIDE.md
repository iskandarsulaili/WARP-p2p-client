# P2P Network DLL - Deployment Guide

**Version:** 1.0.0  
**Last Updated:** November 8, 2025  
**Platform:** Windows 10/11 (x64)

---

## Table of Contents

1. [Overview](#overview)
2. [Prerequisites](#prerequisites)
3. [Deployment Methods](#deployment-methods)
   - [Method 1: WARP Patcher (Recommended)](#method-1-warp-patcher-recommended)
   - [Method 2: Manual DLL Injection](#method-2-manual-dll-injection)
4. [DLL Dependencies](#dll-dependencies)
5. [Configuration Setup](#configuration-setup)
6. [Coordinator Server Deployment](#coordinator-server-deployment)
7. [Security Considerations](#security-considerations)
8. [Testing Deployment](#testing-deployment)
9. [Troubleshooting](#troubleshooting)

---

## Overview

This guide explains how to deploy the P2P Network DLL to end-users and integrate it with the Ragnarok Online client.

### Current Status

✅ **P2P DLL is already built and deployed** to `d:\RO\client\`
✅ **WARP Patcher is available** in `d:\RO\patcher\WARP-p2p-client\win32\WARP.exe`
✅ **All dependencies are present** in the client directory

This guide covers two deployment methods:

1. **WARP Patcher** (recommended) - Modern NEMO alternative for patching RO clients
2. **Manual DLL Injection** (alternative) - Direct DLL loading without patching

### Deployment Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     End User's PC                            │
│  ┌───────────────────────────────────────────────────────┐  │
│  │  Ragnarok Online Client (d:\RO\client\)              │  │
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

## Prerequisites

### Current Setup (Already Complete)

✅ **P2P DLL Built** - `p2p_network.dll` (568 KB) in `d:\RO\client\`
✅ **All Dependencies** - 7 DLLs deployed to `d:\RO\client\`
✅ **WARP Patcher** - Available in `d:\RO\patcher\WARP-p2p-client\win32\WARP.exe`
✅ **RO Client** - Multiple executables in `d:\RO\client\`

### Still Needed

- **Configuration** - Copy `p2p_config.json` to client directory
- **Coordinator Server** - Running and accessible (see [Coordinator Server Deployment](#coordinator-server-deployment))
- **Visual C++ Redistributable 2015-2022** (x64) - https://aka.ms/vs/17/release/vc_redist.x64.exe (for end users)

### For End Users

- **Windows 10/11** (64-bit)
- **Visual C++ Redistributable 2022** (x64)
- **Internet Connection** - For coordinator and STUN servers
- **Firewall Configuration** - Allow UDP traffic for WebRTC (usually automatic)

---

## Deployment Methods

### Step 1: Configure P2P Patch Options (WARP Patch UI)

The WARP patcher now provides a user interface for configuring P2P and server compatibility options before patching or injection:

- **Enable/Disable P2P Networking:** Toggle P2P (WebRTC) support for the client.
- **Mesh Parameters:** Set the maximum number of mesh peers.
- **Fallback:** Enable or disable fallback to server if P2P fails.
- **Server Compatibility:** Select between legacy and new (hybrid) server/coordinator endpoints.

When you start a patch session (e.g., using `P2P_Session.yml`), the patcher will prompt you for these options. Your selections will be written to `p2p_config.json` and used for the patch/injection process.

All errors and important actions are logged to `patcher.log` and surfaced in the patcher UI.

### Step 2: Patch RO Client

1. **Launch WARP Patcher**

   ```powershell
   cd patcher/WARP-p2p-client/win32
   .\WARP.exe
   ```

2. **Load RO client executable**

   - Click "Load Client"
   - Select your RO client (e.g., `Ragnarok.exe`, `2025-06-04_Speedrun_P2P.exe`)

3. **Configure P2P Options**

   - The patcher will prompt for P2P, mesh, fallback, and server type.
   - Confirm your selections.

4. **Select patches**

   - ✅ Enable "CustomDLL" (loads P2P DLL)
   - ✅ Enable other desired patches

5. **Apply patches**

   - Click "Apply Selected Patches"
   - Save patched client (e.g., `Ragnarok_P2P.exe`)

6. **Verify patch**
   - Check patcher UI and `patcher.log` for success or error messages.
   - Patched client should be created.

### Step 3: Deploy Patched Client

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

| DLL File              | Source       | Purpose                        |
| --------------------- | ------------ | ------------------------------ |
| `p2p_network.dll`     | Build output | Main P2P DLL                   |
| `libssl-3-x64.dll`    | OpenSSL      | SSL/TLS support                |
| `libcrypto-3-x64.dll` | OpenSSL      | Cryptography                   |
| `spdlog.dll`          | vcpkg        | Logging                        |
| `brotlidec.dll`       | vcpkg        | Compression (Boost dependency) |

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
    "api_key": "YOUR_API_KEY_HERE"
  },
  "logging": {
    "level": "INFO",
    "file": "logs/p2p_network.log",
    "max_file_size_mb": 10,
    "max_files": 5,
    "console_output": false
  },
  "zones": {
    "p2p_enabled_zones": ["prontera", "geffen", "payon", "alberta"],
    "fallback_on_failure": true
  }
}
```

### Configuration Security

**⚠️ IMPORTANT:** Never commit sensitive data to version control!

**Sensitive Fields:**

- `security.api_key` - Unique per deployment
- `security.jwt_token` - Generated at runtime
- `turn_servers` - May contain credentials

**Best Practices:**

1. Use environment variables for sensitive data
2. Generate unique API keys per server
3. Rotate credentials regularly
4. Use HTTPS/WSS for all coordinator communication

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

### Patch/Injection Error Handling and User Feedback

- All patcher errors (e.g., missing DLL, config write failure, admin rights) are logged to `patcher.log` in the patcher directory.
- The patcher UI will display error messages and guidance if patching or injection fails.
- If you encounter issues, check both the patcher UI and `patcher.log` for details.

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
