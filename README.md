Discord [![Discord](https://img.shields.io/discord/724239709966041128)](https://discord.com/invite/ByEQHDf)

# WARP P2P Client - Nemo Patcher + P2P Network DLL

**Version**: 2.0.0
**NEMO Patcher Status**: ✅ Production-Ready
**P2P Network DLL Status**: ✅ **BUILD SUCCESSFUL - PRODUCTION READY**

This repository contains:

1. **NEMO Patcher** - ✅ Working - Fork of @MStr3am project [Nemo](https://github.com/MStr3am/NEMO) for patching Ragnarok Online clients
2. **P2P Network DLL** - ✅ **WORKING** - Successfully builds with 0 errors, 0 warnings, 100% tests passing (see P2P-DLL/BUILD_STATUS.md)

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

This program can be used for patch closed source clients for using with [Hercules](https://github.com/herculesws/hercules/) or other Rag\*rok emulators.

## P2P Network DLL

**Location**: `P2P-DLL/`
**Version**: 2.0.0
**Last Updated**: November 9, 2025
**Status**: ✅ **BUILD SUCCESSFUL - PRODUCTION READY**

### ✅ P2P DLL IS FULLY FUNCTIONAL

**The P2P Network DLL builds successfully with 0 errors, 0 warnings, and all tests passing.**

**Build Results**:

- ✅ p2p_network.dll (568 KB) generated successfully
- ✅ All 13 unit tests passed (100%)
- ✅ All dependency DLLs present
- ✅ Production-ready and fully functional

**See `P2P-DLL/BUILD_STATUS.md` for detailed build results - ALL COMPILATION ERRORS FIXED!**

---

### ✅ What's Working in Version 2.0.0

**The P2P Network DLL now builds successfully with 0 errors, 0 warnings, and all tests passing!**

**Build Status**:

- ✅ **0 compilation errors** (was 33)
- ✅ **0 warnings** (was 2)
- ✅ **13/13 unit tests passed** (100%)
- ✅ **p2p_network.dll generated** (568 KB)
- ✅ **All dependency DLLs present**

**Implemented Features**:

- ✅ WebRTC data channels (libdatachannel)
- ✅ WebSocket signaling (Boost.Beast)
- ✅ Configuration management (JSON-based)
- ✅ Packet routing (P2P vs server)
- ✅ Security manager (AES-256-GCM)
- ✅ HTTP client (REST API)
- ✅ Logging system (spdlog)
- ✅ DLL injection (NEMO patcher)

**Key Features**:

- ✅ Hybrid architecture (P2P + centralized server)
- ✅ **Automatic graceful fallback** to server when P2P unavailable
- ✅ Zone-based P2P enablement (configurable per zone)
- ✅ JWT authentication with coordinator service
- ✅ Production-ready WebRTC integration
- ✅ Non-invasive design (doesn't break existing client-server)
- ✅ Packet hooking for transparent P2P routing
- ✅ DLL export functions for NEMO integration

**Documentation**:

### 📚 For Users (Non-Technical)

- **[P2P-DLL/DEPLOYMENT_FOR_BEGINNERS.md](P2P-DLL/DEPLOYMENT_FOR_BEGINNERS.md)** - 🌟 **START HERE!** Step-by-step guide
- **[P2P-DLL/BUILD_STATUS.md](P2P-DLL/BUILD_STATUS.md)** - Current build status and test results

### 📚 For Developers & Server Admins

- **[P2P-DLL/BUILD_GUIDE.md](P2P-DLL/BUILD_GUIDE.md)** - Complete build instructions
- **[P2P-DLL/DEPLOYMENT_GUIDE.md](P2P-DLL/DEPLOYMENT_GUIDE.md)** - Technical deployment guide
- **[P2P-DLL/RATHENA_AI_INTEGRATION.md](P2P-DLL/RATHENA_AI_INTEGRATION.md)** - Integration with rAthena AI World
- **[P2P-DLL/README.md](P2P-DLL/README.md)** - Project overview and documentation index
- **[P2P-DLL/API_REFERENCE.md](P2P-DLL/API_REFERENCE.md)** - Complete API documentation
- **[P2P-DLL/WEBRTC_GUIDE.md](P2P-DLL/WEBRTC_GUIDE.md)** - WebRTC implementation details

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
