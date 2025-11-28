# WARP P2P-DLL Patching Guide

> Comprehensive guide for patching Ragnarok Online clients with P2P network DLL support using WARP.exe

---

## Table of Contents

- [Overview](#overview)
  - [What This Patching System Does](#what-this-patching-system-does)
  - [Why WARP.exe Instead of Other Patchers](#why-warpexe-instead-of-other-patchers)
  - [How the CustomDLL Patch Works](#how-the-customdll-patch-works)
- [Prerequisites](#prerequisites)
  - [Obtaining WARP.exe](#obtaining-warpexe)
  - [Required Files](#required-files)
  - [Linux Requirements](#linux-requirements)
- [Quick Start](#quick-start)
  - [Windows Quick Start](#windows-quick-start)
  - [Linux Quick Start](#linux-quick-start)
- [Detailed Usage](#detailed-usage)
  - [Script Parameters](#script-parameters)
  - [Session Files](#session-files)
  - [Customizing P2P_DLLSpec.yml](#customizing-p2p_dllspecyml)
- [Troubleshooting](#troubleshooting)
  - [Common Errors](#common-errors)
  - [Verifying the Patch](#verifying-the-patch)
  - [Client Startup Issues](#client-startup-issues)
- [File Reference](#file-reference)
  - [P2P_DLLSpec.yml Format](#p2p_dllspecyml-format)
  - [Session File Structure](#session-file-structure)
  - [Directory Structure](#directory-structure)
- [Migration from Old Patchers](#migration-from-old-patchers)

---

## Overview

### What This Patching System Does

This patching system modifies a Ragnarok Online (RO) client executable to automatically load the `p2p_network.dll` library at startup. The modification is achieved by adding entries to the client's **Import Address Table (IAT)**, which tells Windows to load the specified DLL when the executable starts.

When the patched client launches:
1. Windows reads the Import Table and loads all required DLLs
2. `p2p_network.dll` is loaded as part of this process
3. The `P2P_Initialize` function is resolved and called
4. The P2P networking layer initializes before the game client begins normal operation

### Why WARP.exe Instead of Other Patchers

Previous attempts to patch the RO client's Import Table using other tools encountered significant issues:

| Patcher | Issue |
|---------|-------|
| **pefile** (Python) | Cannot expand the Import Directory Table when there's insufficient space in the PE headers |
| **LIEF** | Produces clients that fail with Windows error `0xc0000142` (DLL initialization failure) |
| **Detours** | Implementation incomplete; requires manual hooking code |

**WARP.exe** solves these problems through its `CustomDLL.qjs` patch, which:
- Properly handles Import Table expansion
- Correctly manages PE section alignment
- Maintains executable integrity and compatibility
- Has been battle-tested with thousands of RO clients

### How the CustomDLL Patch Works

The CustomDLL patch performs these operations on the target executable:

1. **Parses the DLL specification** from [`Inputs/P2P_DLLSpec.yml`](Inputs/P2P_DLLSpec.yml) to determine which DLLs and functions to add
2. **Expands the Import Directory Table** by creating additional space in the executable
3. **Adds new import entries** for each specified DLL and its exported functions
4. **Updates PE headers** to reflect the modified import structure
5. **Preserves existing imports** to maintain compatibility with all original dependencies

---

## Prerequisites

### Obtaining WARP.exe

WARP is a free, open-source patcher tool developed by Neo-Mind specifically for Ragnarok Online clients.

**Download Location:** [https://github.com/Neo-Mind/WARP](https://github.com/Neo-Mind/WARP)

1. Navigate to the [Releases](https://github.com/Neo-Mind/WARP/releases) page
2. Download the latest release archive (e.g., `WARP_v*.zip`)
3. Extract the contents to the `win32/` directory within this project:

```
WARP-p2p-client/
├── win32/
│   ├── WARP.exe           # GUI application
│   ├── WARP_console.exe   # Console/CLI application
│   ├── Patches/           # Built-in patch scripts
│   │   └── CustomDLL.qjs  # The patch we use
│   ├── Inputs/            # WARP's default input directory
│   └── ...                # Other WARP files
├── Inputs/
│   └── P2P_DLLSpec.yml    # Our DLL specification
├── P2P_Session.yml
├── patch_with_warp.ps1
└── patch_with_warp.sh
```

### Required Files

Ensure these files are in place before patching:

| File | Location | Purpose |
|------|----------|---------|
| `WARP.exe` or `WARP_console.exe` | `win32/` | The patcher executable |
| `P2P_DLLSpec.yml` | `Inputs/` | Specifies the DLL and functions to import |
| `P2P_Session.yml` | Root directory | Defines which patches to apply |
| Your RO client | Anywhere | The executable to be patched |

### Linux Requirements

For Linux users, **Wine** is required to run WARP.exe:

```bash
# Ubuntu/Debian
sudo apt-get install wine

# Fedora
sudo dnf install wine

# Arch Linux
sudo pacman -S wine

# Verify installation
wine --version
```

> **Note:** Wine 5.0 or later is recommended for best compatibility.

---

## Quick Start

### Windows Quick Start

**Console Mode (Recommended for automation):**
```powershell
.\patch_with_warp.ps1 -InputExe "path\to\client.exe" -OutputExe "path\to\client_patched.exe" -ConsoleMode
```

**GUI Mode (For manual configuration):**
```powershell
.\patch_with_warp.ps1 -InputExe "path\to\client.exe" -OutputExe "path\to\client_patched.exe"
```

**Example with actual paths:**
```powershell
# Patch a client in the current directory
.\patch_with_warp.ps1 -InputExe ".\ragexe.exe" -OutputExe ".\ragexe_p2p.exe" -ConsoleMode

# Patch a client with full paths
.\patch_with_warp.ps1 -InputExe "C:\RO\client.exe" -OutputExe "C:\RO\client_patched.exe" -ConsoleMode
```

### Linux Quick Start

**Console Mode:**
```bash
./patch_with_warp.sh -i path/to/client.exe -o path/to/client_patched.exe -c
```

**GUI Mode:**
```bash
./patch_with_warp.sh -i path/to/client.exe -o path/to/client_patched.exe
```

**Example with actual paths:**
```bash
# Make script executable (first time only)
chmod +x patch_with_warp.sh

# Patch a client
./patch_with_warp.sh -i ./ragexe.exe -o ./ragexe_p2p.exe -c

# With full paths
./patch_with_warp.sh -i /home/user/RO/client.exe -o /home/user/RO/client_patched.exe -c
```

---

## Detailed Usage

### Script Parameters

#### PowerShell Script ([`patch_with_warp.ps1`](patch_with_warp.ps1))

| Parameter | Required | Default | Description |
|-----------|----------|---------|-------------|
| `-InputExe` | Yes | — | Path to the input RO client executable to patch |
| `-OutputExe` | Yes | — | Path where the patched executable will be saved |
| `-SessionFile` | No | `P2P_Session.yml` | YAML session file defining patches to apply |
| `-ConsoleMode` | No | `$false` | Use `WARP_console.exe` instead of GUI mode |

**Usage Examples:**
```powershell
# Basic usage with console mode
.\patch_with_warp.ps1 -InputExe "client.exe" -OutputExe "client_patched.exe" -ConsoleMode

# Custom session file
.\patch_with_warp.ps1 -InputExe "client.exe" -OutputExe "client_patched.exe" -SessionFile "P2P_Patch_Session.yml" -ConsoleMode

# GUI mode for interactive patching
.\patch_with_warp.ps1 -InputExe "client.exe" -OutputExe "client_patched.exe"
```

#### Bash Script ([`patch_with_warp.sh`](patch_with_warp.sh))

| Flag | Long Form | Required | Default | Description |
|------|-----------|----------|---------|-------------|
| `-i` | `--input` | Yes | — | Path to input RO client executable |
| `-o` | `--output` | Yes | — | Path for output patched executable |
| `-s` | `--session` | No | `P2P_Session.yml` | Session YAML file to use |
| `-c` | `--console` | No | `false` | Use console mode instead of GUI |
| `-h` | `--help` | No | — | Display help message |

**Usage Examples:**
```bash
# Basic usage with console mode
./patch_with_warp.sh -i client.exe -o client_patched.exe -c

# Long-form options
./patch_with_warp.sh --input client.exe --output client_patched.exe --console

# Custom session file
./patch_with_warp.sh -i client.exe -o client_patched.exe -s P2P_Patch_Session.yml -c

# Display help
./patch_with_warp.sh --help
```

### Session Files

Session files define which patches WARP should apply. Two session files are available:

#### [`P2P_Session.yml`](P2P_Session.yml) (Default)

The default session includes:
- **P2PConfigManager** - P2P-specific configuration patch
- **CustomDLL** - Loads `p2p_network.dll` (our main patch)
- **MultiGRFs** - Enables loading multiple GRF files
- **IncrZoom** - Allows increased zoom levels
- **SkipServiceSelect** - Bypasses service selection screen
- **NoHardCodedAddr** - Removes hardcoded server addresses
- **EnableDnsSupport** - Enables DNS hostname resolution

Use this for a complete patching experience with common quality-of-life improvements.

#### [`P2P_Patch_Session.yml`](P2P_Patch_Session.yml) (Minimal)

A minimal session that only applies:
- **CustomDLL** - Just the P2P DLL patch

Use this when you want to preserve other aspects of your client unchanged.

### Customizing P2P_DLLSpec.yml

The DLL specification file at [`Inputs/P2P_DLLSpec.yml`](Inputs/P2P_DLLSpec.yml) defines which DLLs and functions to add to the Import Table.

**Current Configuration:**
```yaml
# P2P Network DLL - Main networking library
- Name: p2p_network.dll
  Funcs:
    # Primary initialization function
    - Name: P2P_Initialize
```

**To add additional DLLs or functions:**
```yaml
# Multiple DLLs example
- Name: p2p_network.dll
  Funcs:
    - Name: P2P_Initialize
    - Name: P2P_Connect
    - Name: P2P_SendPacket

- Name: custom_hooks.dll
  Funcs:
    - Name: InitHooks
```

**Important Notes:**
- The DLL must exist in the client directory when the patched executable runs
- Function names must match exactly with the DLL's exported symbols
- Each function listed will be resolved at load time (ImportByName)

---

## Troubleshooting

### Common Errors

#### "WARP directory not found"
```
✗ WARP directory not found: /path/to/WARP-p2p-client/win32
```
**Solution:** Download WARP from [GitHub](https://github.com/Neo-Mind/WARP) and extract to the `win32/` directory.

#### "WARP_console.exe not found"
```
✗ WARP_console.exe not found in: /path/to/win32
```
**Solution:** Ensure you downloaded the full WARP release, not just individual files. Both `WARP.exe` and `WARP_console.exe` should be present.

#### "Wine is not installed"
```
✗ Wine is not installed or not in PATH
```
**Solution (Linux):** Install Wine using your distribution's package manager:
```bash
sudo apt-get install wine      # Debian/Ubuntu
sudo dnf install wine          # Fedora
sudo pacman -S wine            # Arch
```

#### "Session file not found"
```
✗ Session file not found: /path/to/P2P_Session.yml
```
**Solution:** Ensure [`P2P_Session.yml`](P2P_Session.yml) exists in the `WARP-p2p-client/` directory. If using a custom session file, verify the path is correct.

#### "DLL specification file not found"
```
⚠ DLL specification file not found: /path/to/Inputs/P2P_DLLSpec.yml
```
**Solution:** Ensure [`Inputs/P2P_DLLSpec.yml`](Inputs/P2P_DLLSpec.yml) exists. The CustomDLL patch requires this file.

#### WARP Exit Code Non-Zero
```
✗ WARP patching failed with exit code: 1
```
**Possible Causes:**
- Invalid input executable (corrupted or not a valid PE file)
- Incompatible client version
- Patch conflicts with existing modifications

**Solution:** Try patching an unmodified (clean) client executable.

### Verifying the Patch

After patching, verify that `p2p_network.dll` was added to the Import Table:

**Using PE analysis tools:**
```bash
# Using objdump (Linux)
objdump -p client_patched.exe | grep -A5 "DLL Name: p2p_network.dll"

# Using dumpbin (Windows, Visual Studio)
dumpbin /imports client_patched.exe | findstr "p2p_network"

# Using CFF Explorer (Windows GUI)
# Open client_patched.exe → Import Directory → Look for p2p_network.dll
```

**Expected Output:**
You should see `p2p_network.dll` listed among the imported DLLs, with `P2P_Initialize` as an imported function.

### Client Startup Issues

If the patched client fails to start:

#### Error: "The application failed to start (0xc0000142)"
**Cause:** DLL initialization failure.
**Solutions:**
1. Ensure `p2p_network.dll` exists in the client directory
2. Verify the DLL was compiled for the correct architecture (32-bit for most RO clients)
3. Check that all DLL dependencies are satisfied
4. Run with administrative privileges

#### Error: "Entry point not found" / "Procedure not found"
**Cause:** The DLL doesn't export the expected function.
**Solution:** Verify that `p2p_network.dll` exports `P2P_Initialize`:
```bash
# Linux
objdump -T p2p_network.dll | grep P2P_Initialize

# Windows (dumpbin)
dumpbin /exports p2p_network.dll | findstr P2P_Initialize
```

#### Error: "The specified module could not be found"
**Cause:** Windows cannot find `p2p_network.dll` or its dependencies.
**Solutions:**
1. Copy `p2p_network.dll` to the same directory as the patched client
2. Check for missing DLL dependencies using Dependency Walker or `ldd` (Wine)
3. Install Visual C++ Redistributables if the DLL requires them

#### Client crashes immediately after launch
**Possible Causes:**
1. DLL initialization code has a bug
2. Architecture mismatch (64-bit DLL with 32-bit client)
3. Memory corruption from patch

**Debugging Steps:**
1. Test with a minimal DLL that only exports an empty `P2P_Initialize` function
2. Attach a debugger to see where the crash occurs
3. Check Windows Event Viewer for application errors

---

## File Reference

### P2P_DLLSpec.yml Format

Location: [`Inputs/P2P_DLLSpec.yml`](Inputs/P2P_DLLSpec.yml)

```yaml
# Each list item represents a DLL to import
- Name: <dll_filename>     # The DLL filename (with .dll extension)
  Funcs:                   # List of functions to import from this DLL
    - Name: <function_name>  # Exact function name as exported by DLL
    # Additional functions...

# Multiple DLLs can be specified
- Name: <another_dll>
  Funcs:
    - Name: <function_name>
```

**Field Descriptions:**
| Field | Type | Description |
|-------|------|-------------|
| `Name` (DLL) | String | The filename of the DLL to import (e.g., `p2p_network.dll`) |
| `Funcs` | List | Array of function specifications to import |
| `Name` (Func) | String | The exact export name of the function |

### Session File Structure

Location: [`P2P_Session.yml`](P2P_Session.yml)

```yaml
patches:
  <PatchName>:
    state: true|false      # Enable or disable this patch
    inputs:                # Optional: patch-specific inputs
      $<variable>: <value> # Variable assignments for the patch
```

**Example:**
```yaml
patches:
  CustomDLL:
    state: true
    inputs:
      $customDLL: Inputs/P2P_DLLSpec.yml
  
  MultiGRFs:
    state: true
  
  DisabledPatch:
    state: false
```

**Available Patches (commonly used):**

| Patch Name | Description |
|------------|-------------|
| `CustomDLL` | Adds custom DLLs to Import Table |
| `MultiGRFs` | Enables loading multiple GRF files |
| `IncrZoom` | Increases maximum zoom level |
| `SkipServiceSelect` | Bypasses service selection screen |
| `NoHardCodedAddr` | Removes hardcoded server addresses |
| `EnableDnsSupport` | Allows DNS hostnames for server |
| `P2PConfigManager` | P2P-specific configuration handling |

### Directory Structure

```
WARP-p2p-client/
├── win32/                      # WARP installation directory
│   ├── WARP.exe                # GUI patcher
│   ├── WARP_console.exe        # Console patcher
│   ├── Patches/                # WARP patch scripts
│   │   ├── CustomDLL.qjs       # Custom DLL injection patch
│   │   └── ...
│   ├── Inputs/                 # WARP default inputs directory
│   ├── Support/                # WARP support files
│   └── Outputs/                # WARP default output directory
│
├── Inputs/                     # Our project's input files
│   └── P2P_DLLSpec.yml         # DLL specification for CustomDLL patch
│
├── P2P_Session.yml             # Full session with multiple patches
├── P2P_Patch_Session.yml       # Minimal session (CustomDLL only)
│
├── patch_with_warp.ps1         # PowerShell automation script
├── patch_with_warp.sh          # Bash automation script (Linux/Wine)
│
├── WARP_PATCHING_GUIDE.md      # This documentation file
└── ...
```

---

## Migration from Old Patchers

If you were previously using other patching methods (pefile, LIEF, Detours), you can migrate to the WARP-based solution:

### Files No Longer Needed

These files/scripts from old approaches can be removed:

| Old File | Purpose | Replacement |
|----------|---------|-------------|
| `patch_exe_draft.py` | pefile-based patcher | `patch_with_warp.ps1/sh` |
| `injector.cpp/obj` | Manual DLL injection | Not needed (import table patching) |
| Python pefile scripts | PE manipulation | WARP CustomDLL patch |
| LIEF-based scripts | PE modification | WARP CustomDLL patch |
| Detours integration | Runtime hooking | Not needed |

### Key Differences

| Aspect | Old Approach | WARP Approach |
|--------|--------------|---------------|
| **Method** | Runtime injection or manual PE editing | Import Table modification |
| **DLL Loading** | Manual injection at runtime | Automatic by Windows loader |
| **Reliability** | Issues with some client versions | Consistent across versions |
| **Complexity** | Multiple tools and scripts | Single, unified tool |
| **Maintenance** | Custom code required | Configuration files only |

### Migration Steps

1. **Keep your `p2p_network.dll`** — It works the same way with WARP
2. **Install WARP** in the `win32/` directory
3. **Verify configuration** files are in place:
   - `Inputs/P2P_DLLSpec.yml`
   - `P2P_Session.yml`
4. **Run the new patching script** instead of old tools
5. **Remove old patching tools** after confirming the WARP approach works

---

## Support

For issues with:
- **WARP tool itself:** [WARP GitHub Issues](https://github.com/Neo-Mind/WARP/issues)
- **This patching setup:** Open an issue in the project repository
- **P2P DLL functionality:** Refer to P2P-DLL documentation

---

*Last updated: 2024*