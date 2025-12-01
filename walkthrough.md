# Walkthrough - P2P DLL Patching

## Overview
I have successfully patched the Ragnarok Online client executable to load `p2p_network.dll` without using `WARP.exe`.

## Changes
### Patched Executable
- **File**: `d:\RO\client\2025-06-04_Speedrun_P2P.exe`
- **Method**: Used `lief` (Python library) to add `p2p_network.dll` to the Import Table.
- **Status**: ✅ Patched successfully.

### Build Status
> [!IMPORTANT]
> **Build Success**: The x86 `p2p_network.dll` was successfully built and deployed.
> The client `2025-06-04_Speedrun_P2P.exe` is **x86 (32-bit)** and now has the compatible **x86** DLLs.

I successfully built the x86 DLL using `vcpkg` manifest mode (fixing architecture and dependency issues).

## Verification
1. **Launch**: Run `d:\RO\client\launch_p2p.bat`.
2. **Expected Behavior**: The game should start and load `p2p_network.dll` without errors.
   - The P2P features (QuicTransport, etc.) should be active.
   - Logs will be generated in the client directory (e.g., `p2p_network.log` if enabled).
3. **Patch Verification**: Run `python verify_patch.py` to confirm `p2p_network.dll` is in the Import Table.

## Scripts Created
- `patch_exe_lief.py`: Patched the EXE (using LIEF Builder).
- `verify_patch.py`: Verifies the patch integrity.
- `build_x86_manifest.ps1`: Built the x86 DLL.
- `deploy_x86.ps1`: Deployed the DLLs.
- `launch_p2p.bat`: Launcher script.
