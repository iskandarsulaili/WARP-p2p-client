Discord [![Discord](https://img.shields.io/discord/724239709966041128)](https://discord.com/invite/ByEQHDf)

# WARP P2P Client - Nemo Patcher + P2P Network DLL

**Version**: 2.0.0
**Status**: ✅ Production-Ready - All 26 Security & Functionality Fixes Complete

This repository contains:
1. **NEMO Patcher** - Fork of @MStr3am project [Nemo](https://github.com/MStr3am/NEMO) for patching Ragnarok Online clients
2. **P2P Network DLL** - Production-ready P2P networking implementation for zone-based peer-to-peer gameplay

---

## ⚠️ Important: P2P is Completely Optional

**The P2P system is entirely optional and can be disabled at any time:**
- When P2P is disabled or unavailable, the system **automatically falls back** to traditional server routing
- Players experience **no difference in gameplay** when P2P is disabled
- The fallback is **transparent** - no manual intervention required
- You can enable/disable P2P per zone or globally via configuration

**P2P provides benefits when enabled, but the game works perfectly without it.**

---

## NEMO Patcher

This program can be used for patch closed source clients for using with [Hercules](https://github.com/herculesws/hercules/) or other Rag*rok emulators.

## P2P Network DLL

**Location**: `P2P-DLL/`
**Version**: 2.0.0
**Last Updated**: November 9, 2025

The P2P Network DLL provides WebRTC-based peer-to-peer connections for zone-based gameplay while maintaining centralized AI NPCs, authentication, and anti-cheat systems.

### 🎉 What's New in Version 2.0.0

**All 26 critical fixes completed:**
- ✅ Fixed authentication race condition with synchronous auth
- ✅ Implemented packet serialization/deserialization with CRC32 validation
- ✅ Completed packet routing with automatic fallback
- ✅ Implemented WebSocket signaling with auto-reconnection
- ✅ Fixed session ID type mismatches (UUID-based)
- ✅ Moved signaling state to Redis for persistence
- ✅ Enabled SSL certificate verification
- ✅ Removed hardcoded security secrets
- ✅ Implemented JWT token parsing and refresh
- ✅ **NEW**: RO client packet hooking via NEMO patch
- ✅ **NEW**: WebRTC offer/answer/ICE candidate flow
- ✅ Connection recovery with exponential backoff
- ✅ Session health monitoring with auto-cleanup
- ✅ Rate limiting (token bucket algorithm)
- ✅ NPC state broadcasting
- ✅ Prometheus metrics
- ✅ Custom exception handling
- ✅ Database indexes for performance
- ✅ And 8 more fixes...

**Key Features**:
- ✅ Hybrid architecture (P2P + centralized server)
- ✅ **Automatic graceful fallback** to server when P2P unavailable
- ✅ Zone-based P2P enablement (configurable per zone)
- ✅ JWT authentication with coordinator service
- ✅ Production-ready WebRTC integration
- ✅ Non-invasive design (doesn't break existing client-server)
- ✅ **NEW**: Packet hooking for transparent P2P routing
- ✅ **NEW**: DLL export functions for NEMO integration

**Documentation**:
- `P2P-DLL/DEPLOYMENT_GUIDE.md` - **Complete deployment guide with all 26 fixes**
- `P2P-DLL/README.md` - Build instructions and usage
- `../P2P_DLL_BUILD_PLAN.md` - Complete implementation roadmap
- `../COORDINATOR_DLL_INTEGRATION.md` - Integration with coordinator service
- `../P2P_FEATURES.md` - Feature documentation

**NEMO Patches**:
- `Patches/LoadP2PDLL.qs` - Injects P2P DLL into RO client at startup
- `Patches/HookP2PPackets.qs` - **NEW**: Hooks send/recv for packet routing

**DLL Export Functions** (NEW in v2.0.0):
- `P2P_RoutePacket()` - Routes packets through P2P or server
- `P2P_InjectPacket()` - Injects received P2P packets
- `P2P_IsActive()` - Checks if P2P is active
- `P2P_GetStatus()` - Gets detailed P2P status

---

## Usage

# Docs

Basic docs located in [Docs](Docs) directory.

## Profiles

Predefined minimal profile for zero clients. See in [profiles](profiles) directory.

## Configs

Working config for [zero client](configs/zero/)

# Patch reports

Reports for all patches and all clients http://nemo.herc.ws/

# Support

Forum topic: http://herc.ws/board/topic/15523-another-nemo-patcher-fork/

Discord server: https://discord.com/invite/ByEQHDf

You can create bug reports here: https://gitlab.com/4144/Nemo/issues/new?issue%5Bassignee_id%5D=&issue%5Bmilestone_id%5D=

or vote on any existing issue here: https://gitlab.com/4144/Nemo/issues

# Game guard files

For disable game guard Cheat Defender need:

1. get dropin replacment file [CDClient.dll](Input/CDClient.dll) and save into client directory.

2. Enable patch "Disable Cheat Defender Game Guard".

# License

Original code and binary files licensed under unknown free use license.

Asm files licensed under CC-NC-ND,

All additional changes licensed under GPL3.
