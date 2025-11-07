Discord [![Discord](https://img.shields.io/discord/724239709966041128)](https://discord.com/invite/ByEQHDf)

# WARP P2P Client - Nemo Patcher + P2P Network DLL

This repository contains:
1. **NEMO Patcher** - Fork of @MStr3am project [Nemo](https://github.com/MStr3am/NEMO) for patching Ragnarok Online clients
2. **P2P Network DLL** - Production-ready P2P networking implementation for zone-based peer-to-peer gameplay

---

## NEMO Patcher

This program can be used for patch closed source clients for using with [Hercules](https://github.com/herculesws/hercules/) or other Rag*rok emulators.

## P2P Network DLL

**Location**: `P2P-DLL/`

The P2P Network DLL provides WebRTC-based peer-to-peer connections for zone-based gameplay while maintaining centralized AI NPCs, authentication, and anti-cheat systems.

**Key Features**:
- ✅ Hybrid architecture (P2P + centralized server)
- ✅ Graceful fallback to server when P2P unavailable
- ✅ Zone-based P2P enablement
- ✅ JWT authentication with coordinator service
- ✅ Production-ready WebRTC integration
- ✅ Non-invasive design (doesn't break existing client-server)

**Documentation**:
- `P2P-DLL/README.md` - Build instructions and usage
- `../P2P_DLL_BUILD_PLAN.md` - Complete implementation roadmap
- `../COORDINATOR_DLL_INTEGRATION.md` - Integration with coordinator service
- `../P2P_FEATURES.md` - Feature documentation

**NEMO Patch**: `Patches/LoadP2PDLL.qs` - Injects P2P DLL into RO client at startup

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
