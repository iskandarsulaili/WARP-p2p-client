# P2P Status Overlay Guide

<div align="center">

**In-Game Network Status Display for P2P DLL**

[![DirectX 9](https://img.shields.io/badge/DirectX-9-blue)]()
[![Platform](https://img.shields.io/badge/platform-Windows%20x86-green)]()

</div>

---

## 📋 Table of Contents

- [Overview](#overview)
- [Display Modes](#display-modes)
- [Usage](#usage)
- [API Reference](#api-reference)
- [Technical Details](#technical-details)
- [Building with Overlay](#building-with-overlay)
- [Troubleshooting](#troubleshooting)

---

## 🎯 Overview

The **P2P Status Overlay** is an in-game visual display system that shows real-time P2P network connection status directly within the Ragnarok Online client. It provides at-a-glance information about your P2P connection without needing to check external logs or tools.

### What It Does

- **Displays P2P connection status** in the top-left corner of the game window
- **Shows network metrics** including peer count, latency, and packet loss
- **Provides debug information** for troubleshooting connection issues
- **Cycles through display modes** using the F9 hotkey

### How It Works

The overlay system hooks into the game's DirectX 9 rendering pipeline using the **EndScene** method. This allows it to draw text directly onto the game screen after all other rendering is complete, ensuring the overlay is always visible on top of the game graphics.

```
┌─────────────────────────────────────────────┐
│ Game Window                                  │
│ ┌───────────────────┐                        │
│ │ === P2P Status ===│ ← Overlay (top-left)   │
│ │ Status: Connected │                        │
│ │ Peers: 5          │                        │
│ │ Ping: 45ms        │                        │
│ └───────────────────┘                        │
│                                              │
│           [Game Content]                     │
│                                              │
└─────────────────────────────────────────────┘
```

---

## 📊 Display Modes

The overlay supports three display modes, each providing different levels of detail. Press **F9** to cycle through modes.

### Mode 1: Basic (Default)

A minimal status indicator showing only connection state.

```
┌──────────────────┐
│ P2P: Connected   │   (Green text when connected)
└──────────────────┘

┌──────────────────┐
│ P2P: Disconnected│   (Red text when disconnected)
└──────────────────┘
```

**Use Case:** Normal gameplay when you just want to verify P2P is working without distraction.

### Mode 2: Connection

Shows connection details including peer count and network quality metrics.

```
┌────────────────────┐
│ === P2P Status === │   (Yellow title)
│ Status: Connected  │   (Green/Red based on state)
│ Peers: 5           │   (White text)
│ Ping: 45ms         │   (White text)
│ Loss: 0.5%         │   (White text)
└────────────────────┘
```

**Metrics Shown:**
| Metric | Description |
|--------|-------------|
| **Status** | Connected or Disconnected |
| **Peers** | Number of P2P peers in current zone |
| **Ping** | Average latency to peers in milliseconds |
| **Loss** | Packet loss percentage |

**Use Case:** When you want to monitor connection quality during gameplay or diagnose lag issues.

### Mode 3: Debug

Full technical information for advanced troubleshooting.

```
┌─────────────────────────────────────┐
│ === P2P Debug Info ===              │   (Yellow title)
│ Status: Connected                   │   (Green/Red)
│ Sent: 1.23 MB                       │   (White text)
│ Recv: 2.45 MB                       │   (White text)
│ Pkts Sent: 1234                     │   (White text)
│ Pkts Recv: 2345                     │   (White text)
│ Pkts Lost: 12                       │   (White text)
│ Latency: 45.0ms                     │   (White text)
│ Bitrate: 2048.0 kbps                │   (White text)
│ Coord: https://api.example.com      │   (Gray text)
└─────────────────────────────────────┘
```

**Metrics Shown:**
| Metric | Description |
|--------|-------------|
| **Status** | Connected or Disconnected |
| **Sent** | Total bytes sent (human-readable: B, KB, MB, GB) |
| **Recv** | Total bytes received |
| **Pkts Sent** | Total packets sent |
| **Pkts Recv** | Total packets received |
| **Pkts Lost** | Total packets lost |
| **Latency** | Average latency in milliseconds (1 decimal) |
| **Bitrate** | Current bitrate in kbps |
| **Coord** | Coordinator server URL |

**Use Case:** Diagnosing network issues, measuring bandwidth usage, or verifying coordinator connectivity.

---

## 🎮 Usage

### Keyboard Controls

| Key | Action |
|-----|--------|
| **F9** | Cycle through overlay modes: Basic → Connection → Debug → Basic |

### Programmatic Control

You can also control the overlay via the DLL's exported API functions:

```cpp
// Enable or disable the overlay
extern "C" __declspec(dllimport) void P2P_SetOverlayEnabled(int enabled);

// Cycle to the next overlay mode (same as pressing F9)
extern "C" __declspec(dllimport) void P2P_CycleOverlayMode();
```

### Example: Controlling Overlay from External Application

```cpp
#include <windows.h>

typedef void (*SetOverlayEnabled_t)(int);
typedef void (*CycleOverlayMode_t)(void);

int main() {
    HMODULE dll = LoadLibraryA("p2p_network.dll");
    if (!dll) return 1;

    auto SetOverlayEnabled = (SetOverlayEnabled_t)GetProcAddress(dll, "P2P_SetOverlayEnabled");
    auto CycleOverlayMode = (CycleOverlayMode_t)GetProcAddress(dll, "P2P_CycleOverlayMode");

    // Disable overlay
    SetOverlayEnabled(0);

    // Enable overlay
    SetOverlayEnabled(1);

    // Cycle to next mode
    CycleOverlayMode();

    FreeLibrary(dll);
    return 0;
}
```

### Example: AutoHotkey Script

```ahk
; Toggle overlay with Ctrl+F9
^F9::
    DllCall("p2p_network.dll\P2P_SetOverlayEnabled", "Int", toggle := !toggle)
return
```

### Screen Position

The overlay is rendered at a fixed position:
- **X Position:** 10 pixels from left edge
- **Y Position:** 10 pixels from top edge
- **Line Height:** 16 pixels per line

---

## 📚 API Reference

### P2P_SetOverlayEnabled

Enable or disable the overlay display.

```cpp
extern "C" __declspec(dllexport) void P2P_SetOverlayEnabled(int enabled);
```

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| `enabled` | `int` | Non-zero (1) to enable, zero (0) to disable |

**Returns:** None (void)

**Example:**
```cpp
P2P_SetOverlayEnabled(1);  // Enable overlay
P2P_SetOverlayEnabled(0);  // Disable overlay
```

**Notes:**
- The overlay is enabled by default when the DLL loads
- Disabling the overlay stops all rendering but keeps the DirectX hooks active
- The keyboard hook (F9) remains active even when overlay is disabled

---

### P2P_CycleOverlayMode

Programmatically cycle to the next overlay display mode.

```cpp
extern "C" __declspec(dllexport) void P2P_CycleOverlayMode();
```

**Parameters:** None

**Returns:** None (void)

**Example:**
```cpp
P2P_CycleOverlayMode();  // Cycle: BASIC → CONNECTION → DEBUG → BASIC
```

**Notes:**
- Equivalent to pressing F9
- Cycles through: BASIC → CONNECTION → DEBUG → BASIC
- Works even if overlay is disabled (mode is remembered when re-enabled)

---

### OverlayMode Enum

Internal enumeration defining display modes.

```cpp
namespace P2P {

enum class OverlayMode {
    BASIC,      // Simple "P2P: Connected/Disconnected" display
    CONNECTION, // Shows peer count, latency, packet loss
    DEBUG       // Full technical details including bandwidth
};

}
```

| Value | Integer | Description |
|-------|---------|-------------|
| `BASIC` | 0 | Minimal status indicator |
| `CONNECTION` | 1 | Connection quality metrics |
| `DEBUG` | 2 | Full technical information |

---

## 🔧 Technical Details

### DirectX 9 Hook Architecture

The overlay uses Microsoft Detours to hook DirectX 9 functions:

```
┌─────────────────────────────────────────────────────────┐
│                    RO Client Process                     │
│  ┌─────────────────────────────────────────────────────┐│
│  │                    Game Loop                         ││
│  │  ┌───────────┐    ┌───────────┐    ┌─────────────┐  ││
│  │  │  Update   │───▶│  Render   │───▶│  EndScene   │  ││
│  │  └───────────┘    └───────────┘    └──────┬──────┘  ││
│  │                                           │         ││
│  │                                    ┌──────▼──────┐  ││
│  │                                    │ Hooked      │  ││
│  │                                    │ EndScene    │  ││
│  │                                    │ ┌─────────┐ │  ││
│  │                                    │ │Overlay  │ │  ││
│  │                                    │ │Render() │ │  ││
│  │                                    │ └─────────┘ │  ││
│  │                                    │      │      │  ││
│  │                                    │      ▼      │  ││
│  │                                    │ Original    │  ││
│  │                                    │ EndScene    │  ││
│  │                                    └─────────────┘  ││
│  └─────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────┘
```

**Hooked Functions:**

| Function | VTable Index | Purpose |
|----------|--------------|---------|
| `EndScene` | 42 | Render overlay after game content |
| `Reset` | 16 | Handle device reset (fullscreen toggle, etc.) |

**Hook Installation:**

```cpp
// Get device VTable
void** vtable = *reinterpret_cast<void***>(device);
g_original_endscene = reinterpret_cast<EndScene_t>(vtable[42]);
g_original_reset = reinterpret_cast<Reset_t>(vtable[16]);

// Install hooks with Detours
DetourTransactionBegin();
DetourUpdateThread(GetCurrentThread());
DetourAttach(&(PVOID&)g_original_endscene, Hooked_EndScene);
DetourAttach(&(PVOID&)g_original_reset, Hooked_Reset);
DetourTransactionCommit();
```

### Thread Safety

The overlay system is designed with thread safety in mind:

| Component | Protection | Description |
|-----------|------------|-------------|
| `current_mode_` | `std::atomic<OverlayMode>` | Mode can be changed from any thread |
| `enabled_` | `std::atomic<bool>` | Enable/disable from any thread |
| `initialized_` | `std::atomic<bool>` | Initialization state |
| `impl_` data | `std::mutex` | Protects font and device pointers |

**Thread Context:**
- **Render()** is called from the game's render thread (via hooked EndScene)
- **CycleMode()** is called from the keyboard hook thread (WH_KEYBOARD_LL)
- **SetEnabled()** can be called from any thread via the DLL API

### Resource Management

The overlay properly handles DirectX device reset scenarios (fullscreen toggle, Alt+Tab, etc.):

```cpp
void OverlayRenderer::OnDeviceReset(IDirect3DDevice9* device, bool before) {
    if (before) {
        // Release resources before reset
        if (impl_->font) {
            impl_->font->OnLostDevice();
        }
    } else {
        // Restore resources after reset
        if (impl_->font) {
            impl_->font->OnResetDevice();
        }
    }
}
```

### Keyboard Hook

The F9 key is detected using a Windows low-level keyboard hook:

```cpp
HHOOK hook_handle_ = SetWindowsHookExW(
    WH_KEYBOARD_LL,       // Hook type
    LowLevelKeyboardProc, // Callback function
    dll_module_,          // DLL handle
    0                     // All threads
);
```

**Key Detection:**
```cpp
if (wParam == WM_KEYDOWN && kbd->vkCode == VK_F9) {
    OverlayRenderer::GetInstance().CycleMode();
}
```

---

## 🏗️ Building with Overlay

### DirectX SDK Requirements

The overlay requires DirectX 9 SDK components:

| Component | Library | Header |
|-----------|---------|--------|
| Direct3D 9 | `d3d9.lib` | `<d3d9.h>` |
| D3DX 9 | `d3dx9.lib` | `<d3dx9.h>` |

**Note:** D3DX9 is part of the legacy DirectX SDK (June 2010). It's not included in the Windows SDK.

### CMake Configuration

The overlay is automatically included when building the P2P DLL. Relevant CMake configuration:

```cmake
# Source files include overlay
set(SOURCES
    # ... other sources ...
    src/overlay/OverlayRenderer.cpp
    src/overlay/KeyboardHook.cpp
)

# Header files include overlay
set(HEADERS
    # ... other headers ...
    include/overlay/OverlayRenderer.h
    include/overlay/KeyboardHook.h
)

# Link DirectX 9 libraries
target_link_libraries(p2p_network PRIVATE
    # ... other libraries ...
    d3d9
    d3dx9
)
```

### x86 (32-bit) Build Requirement

The overlay must be built as **32-bit (x86)** to match the Ragnarok Online client:

```powershell
# Configure for 32-bit build
cmake .. -G "Visual Studio 17 2022" -A Win32 `
    -DCMAKE_TOOLCHAIN_FILE=D:/vcpkg/scripts/buildsystems/vcpkg.cmake `
    -DVCPKG_TARGET_TRIPLET=x86-windows
```

**Important:** Ragnarok Online clients are 32-bit applications. The DLL must match the target architecture.

### vcpkg Dependencies

The Detours library is required for hooking. Install via vcpkg:

```powershell
vcpkg install detours:x86-windows
```

---

## 🔍 Troubleshooting

### Overlay Not Appearing

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| No overlay at all | DLL failed to load | Check `p2p_dll.log` for initialization errors |
| No overlay at all | DirectX hook failed | Verify DirectX 9 is being used by the client |
| No overlay at all | Overlay disabled | Call `P2P_SetOverlayEnabled(1)` or check config |
| Overlay disappeared | Device reset | Overlay should auto-recover; check logs if not |
| Text is garbled | Font creation failed | Check DirectX SDK installation |

**Check DLL Log:**
```powershell
Get-Content p2p_dll.log -Tail 20
```

**Expected Log Messages:**
```
[INFO] Overlay renderer initialized
[INFO] Keyboard hook installed (F9 to cycle overlay modes)
[INFO] DirectX hooks installed successfully
```

### F9 Not Responding

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| F9 doesn't cycle modes | Keyboard hook not installed | Check for `SetWindowsHookExW` errors in log |
| F9 doesn't cycle modes | Another app consuming F9 | Check for conflicting keybinds |
| F9 works but no visual change | Overlay disabled | Enable with `P2P_SetOverlayEnabled(1)` |

**Verify Hook Installation:**
```powershell
# Look for hook installation message
Select-String "Keyboard hook installed" p2p_dll.log
```

### Performance Considerations

The overlay is designed to have minimal performance impact:

| Aspect | Impact | Notes |
|--------|--------|-------|
| CPU Usage | Very Low | Text rendering is lightweight |
| GPU Usage | Minimal | Single DrawText call per frame |
| Memory | ~1-2 MB | Font resources and buffers |
| Frame Time | <0.1ms | Negligible impact on FPS |

**If experiencing performance issues:**

1. Try using BASIC mode (less text to render)
2. Disable overlay during demanding scenarios
3. Check if other overlays (Steam, Discord) are conflicting

### Common Error Messages

| Error | Meaning | Solution |
|-------|---------|----------|
| `Failed to install keyboard hook` | Windows hook installation failed | Run as administrator or check antivirus |
| `Failed to create DirectX font` | D3DXCreateFontA failed | Verify D3DX9 DLLs are present |
| `Failed to install DirectX hooks` | Detours hook failed | Check if client is using DirectX 9 |

### Diagnostic Commands

```powershell
# Check if DLL is loaded
Get-Process -Name ragexe | ForEach-Object { $_.Modules } | Where-Object { $_.ModuleName -eq "p2p_network.dll" }

# View recent log entries
Get-Content p2p_dll.log -Tail 50 | Select-String -Pattern "overlay|hook|DirectX"

# Check for errors
Get-Content p2p_dll.log | Select-String -Pattern "ERROR|WARN"
```

---

## 📎 Related Documentation

- **[README.md](README.md)** - Main P2P DLL documentation
- **[API_REFERENCE.md](API_REFERENCE.md)** - Complete API documentation
- **[BUILD_GUIDE.md](BUILD_GUIDE.md)** - Build instructions
- **[DEPLOYMENT_GUIDE.md](DEPLOYMENT_GUIDE.md)** - Deployment guide

---

## 📝 Changelog

### Version 1.0.0 (November 2025)

- Initial overlay implementation
- Three display modes: Basic, Connection, Debug
- F9 hotkey for mode cycling
- DirectX 9 EndScene hooking
- API functions for programmatic control

---

<div align="center">

**Made with ❤️ for the rAthena Community**

</div>