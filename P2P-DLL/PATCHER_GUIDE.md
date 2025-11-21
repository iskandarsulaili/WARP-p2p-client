# P2P Client Patcher Guide

This guide explains how to use the **One-Click Patcher** (`Patcher.exe`) to enable P2P networking features in your Ragnarok Online client.

## What does this do?
The patcher modifies your game executable (`ragexe.exe` or similar) to automatically load the `p2p_network.dll` when the game starts. This DLL provides the high-performance networking features.

## Prerequisites
Before running the patcher, ensure you have the following files in your game folder:
1.  **Your Game Client**: `ragexe.exe` (or your server's specific executable).
2.  **P2P DLL**: `p2p_network.dll`.
3.  **Dependencies**: The following DLLs must also be in the folder (usually provided with the patch):
    *   `spdlog.dll`
    *   `fmt.dll`
    *   `libssl-3.dll`
    *   `libcrypto-3.dll`
    *   `datachannel.dll`
    *   `msquic.dll`
    *   `libsodium.dll`
    *   `lz4.dll`
    *   `brotlidec.dll`
    *   `juice.dll` (if applicable)

## How to Use

1.  **Copy Files**: Copy `Patcher.exe` into your game folder (where `ragexe.exe` is located).
2.  **Run Patcher**: Double-click `Patcher.exe`.
    *   A black console window will appear.
    *   It will search for `ragexe.exe` (or `2025-06-04_Speedrun.exe`).
    *   **Renaming**: It will rename your original executable to `ragexe_original.exe`.
    *   **Installing**: It will replace `ragexe.exe` with a new **Launcher**.
3.  **Finish**: When it says "Patch Complete!", press **Enter** to close the window.
4.  **Play**: Run `ragexe.exe` as usual. The launcher will start the original game and inject the P2P system transparently.

## Verification
To verify the patch worked:
1.  Start your game client (`ragexe.exe`).
2.  Check for a file named `p2p_dll.log` in your game folder. If it exists, the P2P system is running!

## Troubleshooting

### "The application was unable to start correctly (0xc0000142)"
This error means the game failed to initialize the new DLLs.
*   **Solution 1**: Ensure ALL dependency DLLs listed above are in the game folder.
*   **Solution 2**: Install the **Microsoft Visual C++ Redistributable (x86)**.

### "Error: ragexe.exe not found"
The patcher couldn't find your game client.
*   **Solution**: Drag and drop your game executable onto `Patcher.exe` to patch that specific file.

### Uninstalling
To remove the patch:
1.  Delete `ragexe.exe` (the launcher).
2.  Rename `ragexe_original.exe` back to `ragexe.exe`.
