# P2P Network DLL - rAthena AI World Integration Guide

**Last Updated**: November 9, 2025  
**Version**: 2.0.0  
**Status**: ✅ Production Ready

---

## 📋 Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Integration Points](#integration-points)
4. [Coordinator Service Setup](#coordinator-service-setup)
5. [Configuration Examples](#configuration-examples)
6. [AI NPC Interaction](#ai-npc-interaction)
7. [Security Considerations](#security-considerations)
8. [Deployment Scenarios](#deployment-scenarios)
9. [Monitoring and Debugging](#monitoring-and-debugging)

---

## 🎯 Overview

This guide explains how the **P2P Network DLL** integrates with the **rAthena AI World** server to provide:

- **Hybrid networking**: P2P for player-to-player, centralized for AI NPCs
- **Reduced server load**: Offload player movement/chat to P2P connections
- **Maintained AI control**: All AI NPC interactions remain server-controlled
- **Seamless fallback**: Automatic fallback to server routing when P2P unavailable

### Key Principle

**The P2P DLL does NOT replace the rAthena server - it complements it!**

```
┌─────────────────────────────────────────────────────────────────┐
│                        rAthena AI World                          │
│  ┌──────────────────┐  ┌──────────────────┐  ┌──────────────┐  │
│  │   Map Server     │  │  AI NPC Engine   │  │  Coordinator │  │
│  │  (Traditional)   │  │  (LLM-powered)   │  │   Service    │  │
│  └──────────────────┘  └──────────────────┘  └──────────────┘  │
│           │                     │                     │          │
└───────────┼─────────────────────┼─────────────────────┼──────────┘
            │                     │                     │
            │                     │                     │
    ┌───────┴─────────────────────┴─────────────────────┴───────┐
    │                                                             │
    │                    Client with P2P DLL                      │
    │  ┌──────────────────────────────────────────────────────┐  │
    │  │  Packet Router (Smart Routing)                       │  │
    │  │  • Player packets → P2P (if available)               │  │
    │  │  • NPC packets → Server (always)                     │  │
    │  │  • Item/skill packets → Server (always)              │  │
    │  └──────────────────────────────────────────────────────┘  │
    └─────────────────────────────────────────────────────────────┘
```

---

## 🏗️ Architecture

### Component Relationships

#### 1. rAthena Map Server
**Role**: Traditional game server handling:
- AI NPC interactions (LLM-powered conversations)
- Item transactions and inventory
- Skill usage and combat
- Quest management
- Anti-cheat validation

**P2P Integration**: None - operates normally, unaware of P2P

#### 2. P2P Coordinator Service
**Role**: Facilitates P2P connections:
- WebRTC signaling (SDP/ICE exchange)
- Session discovery and matchmaking
- Authentication and authorization
- Connection health monitoring

**Location**: Can run on same server as rAthena or separately

**Technology**: Python FastAPI service (see `services/ai_service/` in rAthena AI World)

#### 3. P2P Network DLL (Client-Side)
**Role**: Manages hybrid networking:
- Establishes P2P connections via coordinator
- Routes packets intelligently (P2P vs server)
- Handles encryption and security
- Automatic fallback to server routing

**Integration**: Injected into RO client via NEMO patcher

---

## 🔌 Integration Points

### 1. Packet Routing Logic

The P2P DLL uses intelligent packet routing:

```cpp
// Pseudo-code from PacketRouter.cpp
bool PacketRouter::RoutePacket(const Packet& packet) {
    // ALWAYS route to server (never P2P):
    if (IsNPCPacket(packet) ||        // AI NPC interactions
        IsItemPacket(packet) ||        // Item transactions
        IsSkillPacket(packet) ||       // Skill usage
        IsQuestPacket(packet) ||       // Quest updates
        IsAuthPacket(packet)) {        // Authentication
        return SendToServer(packet);
    }
    
    // CAN route to P2P (if available):
    if (IsPlayerMovementPacket(packet) ||  // Player walking
        IsChatPacket(packet) ||            // Player chat
        IsEmotePacket(packet)) {           // Emotes
        if (P2PAvailable() && InP2PZone()) {
            return SendToP2P(packet);
        }
    }
    
    // Fallback to server
    return SendToServer(packet);
}
```

**Key Insight**: AI NPC packets ALWAYS go to the server, ensuring LLM-powered NPCs work correctly.

### 2. Zone-Based P2P

P2P is enabled per-zone, configured in `p2p_config.json`:

```json
{
  "zones": {
    "p2p_enabled_zones": [
      "prontera",      // Busy social hub - good for P2P
      "geffen",        // Another busy city
      "payon"          // Trading area
    ],
    "fallback_on_failure": true
  }
}
```

**Recommendation for rAthena AI World**:
- **Enable P2P**: Social hubs, cities, busy areas
- **Disable P2P**: Dungeons with AI NPCs, quest areas, instances

### 3. Coordinator Service Integration

The coordinator service is part of the rAthena AI World stack:

**File**: `services/ai_service/coordinator_service.py` (hypothetical - may need creation)

**Responsibilities**:
- Authenticate clients using rAthena account system
- Manage P2P sessions per zone
- Handle WebRTC signaling
- Monitor connection health

**API Endpoints**:
```
POST   /api/v1/auth/login          - Authenticate client
GET    /api/v1/sessions/discover   - Find P2P sessions in zone
POST   /api/v1/sessions/join       - Join a P2P session
WS     /api/v1/signaling/ws        - WebSocket for SDP/ICE exchange
```

---

## 🚀 Coordinator Service Setup

### Option 1: Integrated with AI Service (Recommended)

If you're already running the rAthena AI Service:

1. **Add coordinator endpoints** to existing FastAPI service
2. **Share authentication** with AI service (same JWT tokens)
3. **Use same Redis** for session state
4. **Deploy together** on same server

**Advantages**:
- Single deployment
- Shared authentication
- Easier maintenance

### Option 2: Standalone Coordinator

Run coordinator as separate service:

1. **Deploy coordinator** on different server/port
2. **Configure separate auth** (or proxy to AI service)
3. **Use dedicated Redis** instance

**Advantages**:
- Independent scaling
- Isolated failures
- Dedicated resources

### Installation Steps (Integrated)

```bash
# Navigate to AI service directory
cd services/ai_service/

# Install additional dependencies
pip install python-socketio aioredis

# Add coordinator module
# (Create coordinator_service.py with endpoints)

# Update main.py to include coordinator routes
# app.include_router(coordinator_router, prefix="/api/v1")

# Restart AI service
systemctl restart rathena-ai-service
```

### Configuration

Add to `services/ai_service/config.yaml`:

```yaml
coordinator:
  enabled: true
  port: 8001
  websocket_path: "/api/v1/signaling/ws"
  
  # Session settings
  max_sessions_per_zone: 10
  max_players_per_session: 50
  session_timeout_seconds: 300
  
  # WebRTC settings
  stun_servers:
    - "stun:stun.l.google.com:19302"
  turn_servers: []  # Add TURN if needed
  
  # Security
  require_authentication: true
  jwt_secret: "${JWT_SECRET}"  # Same as AI service
  
  # Redis for session state
  redis:
    host: "localhost"
    port: 6379
    db: 1  # Different DB from AI service
```

---

## ⚙️ Configuration Examples

### Development Environment

**Client (`p2p_config.json`)**:
```json
{
  "coordinator": {
    "rest_api_url": "http://localhost:8001/api/v1",
    "websocket_url": "ws://localhost:8001/api/v1/signaling/ws",
    "timeout_seconds": 30
  },
  "p2p": {
    "enabled": true,
    "max_peers": 10  // Lower for testing
  },
  "security": {
    "enable_encryption": false,  // Disable for easier debugging
    "enable_authentication": false
  },
  "logging": {
    "level": "debug",  // Verbose logging
    "console_output": true
  }
}
```

**Server (`config.yaml`)**:
```yaml
coordinator:
  enabled: true
  port: 8001
  require_authentication: false  # Easier testing
  
logging:
  level: DEBUG
```

### Production Environment

**Client (`p2p_config.json`)**:
```json
{
  "coordinator": {
    "rest_api_url": "https://your-server.com:8001/api/v1",
    "websocket_url": "wss://your-server.com:8001/api/v1/signaling/ws",
    "timeout_seconds": 30
  },
  "p2p": {
    "enabled": true,
    "max_peers": 50
  },
  "security": {
    "enable_encryption": true,  // REQUIRED in production
    "enable_authentication": true,
    "certificate_validation": true,
    "tls_version": "1.3"
  },
  "logging": {
    "level": "info",  // Less verbose
    "console_output": false,
    "file": "p2p_dll.log"
  },
  "zones": {
    "p2p_enabled_zones": [
      "prontera",
      "geffen",
      "payon",
      "morocc",
      "alberta"
    ],
    "fallback_on_failure": true
  }
}
```

**Server (`config.yaml`)**:
```yaml
coordinator:
  enabled: true
  port: 8001
  require_authentication: true
  jwt_secret: "${JWT_SECRET}"  # Use environment variable
  
  # Production limits
  max_sessions_per_zone: 20
  max_players_per_session: 50
  
  # TURN server for NAT traversal
  turn_servers:
    - url: "turn:your-turn-server.com:3478"
      username: "${TURN_USERNAME}"
      credential: "${TURN_PASSWORD}"
  
security:
  ssl_cert: "/etc/ssl/certs/your-server.crt"
  ssl_key: "/etc/ssl/private/your-server.key"
  
logging:
  level: INFO
  file: "/var/log/rathena/coordinator.log"
```

---

## 🤖 AI NPC Interaction

### How AI NPCs Work with P2P

**Important**: AI NPC interactions are NEVER routed through P2P. They always go to the server.

**Flow**:
```
1. Player clicks on AI NPC
   ↓
2. Client sends NPC interaction packet
   ↓
3. P2P DLL detects NPC packet type
   ↓
4. Packet routed to rAthena server (NOT P2P)
   ↓
5. rAthena AI Service processes with LLM
   ↓
6. Response sent back to client via server
```

**Why?**
- AI NPCs require server-side LLM processing
- NPC state must be centralized
- Anti-cheat validation needed
- Conversation history stored server-side

### Packet Types Always Routed to Server

```cpp
// From PacketRouter.cpp
const std::set<uint16_t> SERVER_ONLY_PACKETS = {
    // NPC packets
    0x0090,  // NPC Click
    0x00B8,  // NPC Next
    0x00B9,  // NPC Close
    0x00A7,  // NPC Input
    0x01D5,  // NPC String Input
    
    // Item packets
    0x00A2,  // Use Item
    0x00A7,  // Drop Item
    0x00F3,  // Pick Up Item
    
    // Skill packets
    0x0113,  // Use Skill
    0x0438,  // Use Skill (new)
    
    // Quest packets
    0x02B3,  // Quest Info
    0x02B4,  // Quest Accept
    
    // ... (full list in source code)
};
```

---

## 🔒 Security Considerations

### 1. Authentication Flow

```
1. Client launches with P2P DLL
   ↓
2. Client authenticates with rAthena (normal login)
   ↓
3. rAthena issues JWT token
   ↓
4. Client sends JWT to coordinator service
   ↓
5. Coordinator validates JWT with rAthena
   ↓
6. Coordinator allows P2P session creation
```

### 2. Packet Validation

**Server-Side** (rAthena):
- Validate ALL packets from clients (even if P2P-routed)
- Check item transactions, skill usage, movement bounds
- Maintain authoritative game state

**Client-Side** (P2P DLL):
- Encrypt P2P packets with AES-256-GCM
- Validate packet signatures
- Rate limiting to prevent spam

### 3. Anti-Cheat Integration

**What P2P Does NOT Affect**:
- Server-side anti-cheat still works normally
- All critical packets go through server validation
- P2P only handles non-critical player-to-player data

**What to Monitor**:
- Unusual P2P traffic patterns
- Clients bypassing P2P routing
- Packet injection attempts

---

## 📊 Deployment Scenarios

### Scenario 1: Small Server (< 100 players)

**Recommendation**: Integrated coordinator, P2P optional

```yaml
# config.yaml
coordinator:
  enabled: true  # Enable but not critical
  max_sessions_per_zone: 5
  max_players_per_session: 20

# Client can disable P2P if coordinator unavailable
```

**Why**: Small servers don't need P2P for performance, but it's nice to have.

### Scenario 2: Medium Server (100-500 players)

**Recommendation**: Integrated coordinator, P2P for busy zones

```yaml
coordinator:
  enabled: true
  max_sessions_per_zone: 10
  max_players_per_session: 50

# Enable P2P in: prontera, geffen, payon
# Disable in: dungeons, instances
```

**Why**: P2P reduces server load in social hubs where players congregate.

### Scenario 3: Large Server (500+ players)

**Recommendation**: Standalone coordinator, P2P required for busy zones

```yaml
coordinator:
  enabled: true
  dedicated_server: true  # Run on separate machine
  max_sessions_per_zone: 20
  max_players_per_session: 50
  
  # Use TURN servers for better NAT traversal
  turn_servers:
    - url: "turn:turn1.example.com:3478"
    - url: "turn:turn2.example.com:3478"
```

**Why**: Large servers benefit significantly from P2P offloading.

---

## 🔍 Monitoring and Debugging

### Metrics to Track

**Server-Side** (Coordinator):
- Active P2P sessions per zone
- Players connected via P2P vs server
- WebRTC connection success rate
- Signaling latency
- Bandwidth usage

**Client-Side** (P2P DLL):
- P2P connection status
- Packet routing decisions (P2P vs server)
- WebRTC connection quality
- Fallback events

### Log Files

**Client**: `p2p_dll.log` in RO client folder
```
[info] P2P Network DLL initialized
[info] Connected to coordinator: https://your-server.com:8001
[info] Joined P2P session in zone: prontera
[debug] Routing player movement to P2P
[debug] Routing NPC interaction to server
```

**Server**: `/var/log/rathena/coordinator.log`
```
[INFO] New P2P session created: zone=prontera, session_id=abc123
[INFO] Player joined session: account_id=12345, session_id=abc123
[DEBUG] WebRTC signaling: SDP offer from client 12345
[DEBUG] WebRTC signaling: ICE candidate from client 12345
```

### Debugging Commands

**Check coordinator status**:
```bash
curl http://localhost:8001/api/v1/health
```

**List active sessions**:
```bash
curl http://localhost:8001/api/v1/sessions/list
```

**View session details**:
```bash
curl http://localhost:8001/api/v1/sessions/abc123
```

---

## 📚 Additional Resources

- **BUILD_GUIDE.md** - How to build the P2P DLL
- **DEPLOYMENT_GUIDE.md** - Technical deployment details
- **DEPLOYMENT_FOR_BEGINNERS.md** - User-friendly deployment guide
- **API_REFERENCE.md** - Complete API documentation
- **WEBRTC_GUIDE.md** - WebRTC implementation details

---

## ✅ Integration Checklist

- [ ] rAthena AI World server running
- [ ] AI Service deployed and accessible
- [ ] Coordinator service configured (integrated or standalone)
- [ ] Redis instance available for session state
- [ ] JWT authentication configured
- [ ] P2P-enabled zones defined
- [ ] Client DLL built and tested
- [ ] NEMO patches applied to client
- [ ] Configuration files distributed to clients
- [ ] Monitoring and logging set up
- [ ] Fallback to server routing tested
- [ ] AI NPC interactions verified (always server-routed)

---

**The P2P Network DLL seamlessly integrates with rAthena AI World to provide hybrid networking while preserving the power of LLM-driven AI NPCs!**

