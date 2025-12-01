<p align="center">
    <img src="Images/logo.png?raw=true" alt="Warp logo" width=128 height=128>
</p>

# Win App Revamp Package
![License](https://img.shields.io/github/license/Neo-Mind/WARP)
![RepoSize](https://img.shields.io/github/repo-size/Neo-Mind/WARP)
![Commit](https://img.shields.io/github/last-commit/Neo-Mind/WARP)
![DiscordInfo](https://img.shields.io/discord/780647066871136266?label=Discord&logo=Discord&logoColor=white)

WARP is a package of tools for Querying and Revamping a 32 bit Windows Application by means of JS (ECMA-262) Scripts.<br>
The core tools were written in C++ utilizing the versatile Qt Framework, while the tools themselves provide APIs extending traditional JS for writing the scripts.

[Wiki](https://github.com/Neo-Mind/WARP/wiki) | [Discord](https://discord.gg/WGeB4wZZgS) | [Issues](https://github.com/Neo-Mind/WARP/issues/new?template=bug_report.md) | [Feature Requests](https://github.com/Neo-Mind/WARP/issues/new?template=feature_request.md) | [Changelog](CHANGELOG.md)
---|---|---|---

## 🚀 WARP.exe-based P2P-DLL Patching (New Method)

We've implemented a new, reliable WARP.exe-based solution for patching game executables with P2P-DLL functionality. This method uses WARP's powerful scripting capabilities to directly modify the executable's import table.

### Quick Start

**Windows:**
```powershell
.\patch_with_warp.ps1
```

**Linux:**
```bash
./patch_with_warp.sh
```

### Features
- ✅ **Direct Import Table Modification** - Properly expands the import directory table
- ✅ **No Runtime Errors** - Avoids 0xc0000142 initialization errors
- ✅ **Clean Integration** - P2P-DLL loads automatically with the game
- ✅ **Cross-Platform Support** - Works on both Windows and Linux

### Documentation
For detailed information about the patching process, troubleshooting, and technical details, see:
- 📖 **[WARP_PATCHING_GUIDE.md](WARP_PATCHING_GUIDE.md)** - Comprehensive patching guide
- 📖 **[P2P_DLL_INJECTOR_GUIDE.md](P2P_DLL_INJECTOR_GUIDE.md)** - DLL injector documentation

### ⚠️ Deprecated Patchers

The following old patching approaches have been **removed** as they were unreliable and caused issues:
- ❌ `patch_exe_draft.py` - pefile-based approach (couldn't expand Import Directory Table)
- ❌ `patch_exe_lief.py` - LIEF-based approach (caused 0xc0000142 errors)
- ❌ `launcher.cpp` / `Launcher.exe` - Indirect launcher/injector workaround
- ❌ `merge_patcher.py` - Complex merge patcher workaround
- ❌ `patcher.py` - Old patcher implementation

**Use the new WARP.exe-based patching scripts instead** - they provide a cleaner, more reliable solution that directly modifies the executable's import table without runtime injection.

---

## What's included
The package follows the file hierarchy as shown below.

```text
WARP/
│
├── README.md        (This readme file)
│
├── LICENSE          (GPL-3.0 license file)
│
├── ICON_attribution (Attribution for the tool icons & the logo)
│
├── Patches.yml      (YAML file describing all the patches)
│
├── Extensions.yml   (YAML file describing all the extensions)
│
├── Settings.yml     (YAML file containing all the tool settings)
│
├── LastSession.yml  (YAML session file from the last patch application)
│
├── Wiki/     (The Wiki's repository)
│
├── Fonts/    (All fonts contained in here are automatically loaded. NovaFlat is used as default.)
│   │
│   ├── NovaFlat-Bold.ttf
│   └── NovaFlat.ttf
│
├── Images/   (Contains all images used by the Tools.)
│   │
│   ├── Wiki (Images used in the Wiki)
│   │
│   ├── Dark_Mode (Overrides used in Dark Mode)
│   │   ├── bold_on.png
│   │   ├── error_header.png
│   │   ├── github_a.png
│   │   ├── github_i.png
│   │   ├── grip.png
│   │   ├── italic_on.png
│   │   ├── query_header.png
│   │   ├── success_header.png
│   │   └── warn_header.png
│   │
│   ├── actns_a.png
│   ├── actns_i.png
│   ├── ascend.png
│   ├── bold_off.png
│   ├── bold_on.png
│   ├── browse_a.png
│   ├── browse_i.png
│   ├── clear_a.png
│   ├── clear_i.png
│   ├── descend.png
│   ├── discord_a.png
│   ├── discord_i.png
│   ├── error_header.png
│   ├── extns_a.png
│   ├── extns_i.png
│   ├── github_a.png
│   ├── github_i.png
│   ├── grip.png
│   ├── info_a.png
│   ├── info_i.png
│   ├── italic_off.png
│   ├── italic_on.png
│   ├── logo.png
│   ├── next_a.png
│   ├── next_i.png
│   ├── prev_a.png
│   ├── prev_i.png
│   ├── query_header.png
│   ├── rcmd_i.png
│   ├── rcmd_s.png
│   ├── search.png
│   ├── success_header.png
│   └── warn_header.png
│
├── Scripts/
│   │
│   ├── Support/        (Contains all scripts which add supporting data & functions for Patches & Extensions.)
│   │   │
│   │   ├── Addons.qjs            (Implements addons to the existing Prototypes)
│   │   ├── AllDebug.qjs          (Implements functions used for debugging)
│   │   ├── AllFuncs.qjs          (Implements supporting functions)
│   │   ├── Class_IPrefix.qjs     (Represents Instruction Prefix)
│   │   ├── Class_Instr.qjs       (Represents Instruction)
│   │   ├── Class_ModRM.qjs       (Represents ModRM byte)
│   │   ├── Class_OpData.qjs      (Represents Operational Data)
│   │   ├── Class_PtrSize.qjs     (Represents Memory Pointer size)
│   │   ├── Class_Register.qjs    (Represents CPU register)
│   │   ├── Class_SIBase.qjs      (Represents SIB byte)
│   │   ├── Constants.qjs         (Commonly used constants)
│   │   ├── Instructions.qjs      (Generic instruction generators)
│   │   ├── Instructions_ST.qjs   (ST based instruction generators)
│   │   └── Instructions_XMM.qjs  (XMM based instruction generators)
│   │
│   ├── Patches/      (Contains all scripts implementing Patches)
│   │
│   ├── Extensions/   (Contains all scripts implementing Extensions)
│   │
│   └── Init/         (Contains all initialization scripts. Gets loaded each time an app is loaded)
│
├── Languages/   (Contains all Language description YAML files)
│                
├── Styles/      (Contains all Styling description YAML files)
│                
├── Inputs/      (Contains all input files for Patches & Extensions here)
│                
├── Outputs/     (Use this folder for generating files from Extensions & Patches)
│
└── <os_specific_folder>/    (Contains the tools along with DLL/SO files)
```

## Supported Platforms
- Windows (Only this version is available as of now but will be extended to other platforms later)

## P2P Network DLL Integration

This project includes support for integrating a custom P2P Network DLL with Ragnarok Online clients using WARP's powerful patching capabilities.

### WARP-Based Patching Method

The WARP.exe-based solution directly modifies the executable's import table to load `P2P-DLL/p2p_network.dll` automatically when the client starts.

**Quick Setup:**
1. Ensure [`P2P-DLL/p2p_network.dll`](WARP-p2p-client/P2P-DLL/p2p_network.dll) exists
2. Run the patching script:
   - Windows: `.\patch_with_warp.ps1`
   - Linux: `./patch_with_warp.sh`
3. The script will automatically create `output.exe` with P2P-DLL integrated

**How It Works:**
- Uses WARP's scripting engine to modify the PE import table
- Adds P2P-DLL as a new import entry
- Properly expands the import directory table
- No runtime injection needed - the DLL loads naturally with the executable

**Configuration Files:**
- `P2P_Patch_Session.yml` - WARP session configuration for patching
- `Patches.yml` - Contains the CustomDLL patch definition
- `Extensions.yml` - WARP extension configurations

### Alternative: DLL Injector (Fallback)

If needed, a C++ DLL injector (`p2p_injector_x86.exe`) is available as a fallback method. See [`P2P_DLL_INJECTOR_GUIDE.md`](WARP-p2p-client/P2P_DLL_INJECTOR_GUIDE.md) for details.

### Troubleshooting

**Common Issues:**
1. **WARP.exe Not Found**: Ensure WARP is properly installed in the Windows-x64 directory
2. **DLL Not Loading**: Verify `P2P-DLL/p2p_network.dll` exists and has proper dependencies
3. **Patching Fails**: Check `build_log.txt` for error messages

**Verification:**
- Check for `output.exe` creation after patching
- Look for P2P-DLL initialization in game logs
- Verify P2P network functionality in-game

For comprehensive documentation, see [`WARP_PATCHING_GUIDE.md`](WARP-p2p-client/WARP_PATCHING_GUIDE.md).
