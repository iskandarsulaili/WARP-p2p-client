# Windows - WARP P2P Client Setup Guide

**Version**: 1.0  
**Date**: 2025-11-07  
**Target**: Windows 10/11  
**Difficulty**: Beginner-Friendly

---

## Table of Contents

1. [Overview](#overview)
2. [Prerequisites & System Requirements](#prerequisites--system-requirements)
3. [Understanding WARP](#understanding-warp)
4. [Step 1: Install Prerequisites](#step-1-install-prerequisites)
5. [Step 2: Download Ragnarok Online Client](#step-2-download-ragnarok-online-client)
6. [Step 3: Download WARP P2P Client](#step-3-download-warp-p2p-client)
7. [Step 4: Configure P2P Settings](#step-4-configure-p2p-settings)
8. [Step 5: Using WARP to Patch Your Client](#step-5-using-warp-to-patch-your-client)
9. [Step 6: Connecting to the Server](#step-6-connecting-to-the-server)
10. [Troubleshooting](#troubleshooting)
11. [Advanced Configuration](#advanced-configuration)
12. [Building WARP from Source (Advanced)](#building-warp-from-source-advanced)

---

## Overview

This guide will help you set up and use the WARP P2P client on Windows to connect to the rAthena AI World server with P2P hosting capabilities.

**What You'll Learn**:
- How to install and configure WARP
- How to enable P2P hosting features
- How to connect to the rAthena AI World server
- How to troubleshoot common issues

**Estimated Time**: 30-60 minutes  
**Skill Level**: Basic Windows knowledge required

---

## Prerequisites & System Requirements

### Hardware Requirements

| Component | Minimum | Recommended (for P2P Hosting) |
|-----------|---------|-------------------------------|
| CPU | Dual-core @ 2.0 GHz | Quad-core @ 3.0+ GHz |
| RAM | 4 GB | 8+ GB |
| Storage | 5 GB free | 10+ GB free |
| Network | 10 Mbps | 100+ Mbps |
| GPU | DirectX 9.0c compatible | DirectX 11+ compatible |

### Software Requirements

- **OS**: Windows 10 (64-bit) or Windows 11
- **DirectX**: DirectX 9.0c or higher
- **.NET Framework**: 4.5 or higher
- **Visual C++ Redistributables**: 2015-2022 (x86 and x64)
- **Ragnarok Online Client**: Official or compatible client executable

### Network Requirements

- **Stable Internet Connection**: Required for server connection
- **Open Ports** (for P2P hosting):
  - Outbound: 8001 (P2P coordinator WebSocket)
  - Inbound: Dynamic ports for WebRTC (configured by Windows Firewall)

---

## Understanding WARP

### What is WARP?

**WARP** (Win App Revamp Package) is a **client patcher tool** for Ragnarok Online. It's NOT the game client itself, but a tool that modifies the game client executable to enable various features and customizations.

### What is WARP P2P Client?

The **WARP P2P Client** is an enhanced version of WARP that includes:
- **P2P Hosting Patches**: Enable peer-to-peer hosting capabilities
- **WebSocket Integration**: Connect to the P2P coordinator service
- **Configuration Tools**: Easy setup for P2P features

### Important Notes

⚠️ **WARP is a Patcher, Not the Game**:
- WARP modifies your existing Ragnarok Online client
- You need a Ragnarok Online client executable (e.g., `Ragexe.exe`)
- WARP creates a patched version of your client with P2P features

⚠️ **Pre-built Executables**:
- The `win32/` directory contains pre-built WARP executables
- These are the WARP patcher tools, not the game client
- You'll use these to patch your RO client

---

## Step 1: Install Prerequisites

### 1.1 Install .NET Framework 4.5+

```powershell
# Check if .NET Framework is installed
Get-ItemProperty "HKLM:\SOFTWARE\Microsoft\NET Framework Setup\NDP\v4\Full" | Select-Object Version
```

If not installed, download from: https://dotnet.microsoft.com/download/dotnet-framework

### 1.2 Install Visual C++ Redistributables

Download and install both x86 and x64 versions:
- **Download**: https://aka.ms/vs/17/release/vc_redist.x86.exe
- **Download**: https://aka.ms/vs/17/release/vc_redist.x64.exe

Run both installers and follow the prompts.

### 1.3 Install DirectX 9.0c

If you don't have DirectX 9.0c:
- **Download**: https://www.microsoft.com/en-us/download/details.aspx?id=35
- Run the installer and follow the prompts

### 1.4 Verify Ragnarok Online Client

Ensure you have a Ragnarok Online client installed:
- **Client Location**: e.g., `C:\Program Files (x86)\Ragnarok Online\`
- **Client Executable**: e.g., `Ragexe.exe` or similar
- **Client Version**: Compatible with rAthena (2018+ clients recommended)

> **Note**: If you don't have a RO client, proceed to **Step 2** to download a compatible client.

---

## Step 2: Download Ragnarok Online Client

### 2.1 Understanding Client Versions

The Ragnarok Online client is the actual game executable that WARP will patch. For the rAthena AI World server with P2P features, you need a compatible client version.

**Recommended Client Versions** (in order of preference):

| Client Date | Type | Compatibility | Stability | Download Source |
|-------------|------|---------------|-----------|-----------------|
| **2018-06-20** | Ragexe/RagexeRE | ⭐ Excellent | ⭐⭐⭐⭐⭐ | NEMO (Primary) |
| **2020-04-01** | Ragexe | ⭐ Excellent | ⭐⭐⭐⭐ | NEMO (Alternative) |
| **2019-05-30** | Ragexe/RagexeRE | ✓ Good | ⭐⭐⭐⭐ | NEMO (Alternative) |

**Why These Versions?**
- **2018-06-20**: Most stable and widely recommended by rAthena community
- **2020-04-01**: Good balance of features and stability
- **2019-05-30**: Alternative with good compatibility

### 2.2 Download from NEMO Repository

**Primary Recommendation: 2018-06-20eRagexe**

1. **Visit NEMO Downloads Page**:
   - URL: http://nemo.herc.ws/downloads/
   - Navigate to the **2018** section

2. **Download the Client Executable**:
   - **For Pre-Renewal**: Download `2018-06-20eRagexe`
   - **For Renewal**: Download `2018-06-20eRagexeRE`

   Direct download links:
   ```
   Pre-Renewal: http://nemo.herc.ws/downloads/2018-06-20eRagexe
   Renewal:     http://nemo.herc.ws/downloads/2018-06-20eRagexeRE
   ```

3. **Alternative: Download Full Client Package**:

   If you need the complete client with data files:
   - **Taiwan RO 2018-01-26**: http://twcdn.gnjoy.com.tw/ragnarok/Client/RO_Restart_180126.zip
   - **Taiwan RO 2020-01-20**: http://twcdn.gnjoy.com.tw/ragnarok/Client/RO_Install_200120.exe

   > **Note**: Full client packages include all game data files (sprites, maps, etc.)

### 2.3 Install the Client

**Option A: Using Downloaded Executable Only**

If you downloaded just the executable (e.g., `2018-06-20eRagexe`):

1. Create a client directory:
   ```powershell
   mkdir "C:\Ragnarok Online"
   ```

2. Place the downloaded executable in the directory:
   ```powershell
   # Rename the downloaded file to Ragexe.exe
   Rename-Item "C:\Downloads\2018-06-20eRagexe" "C:\Ragnarok Online\Ragexe.exe"
   ```

3. **You'll need game data files** (GRF files, data folder):
   - Download from your server administrator
   - Or use data files from an existing RO installation
   - Required files: `data.grf`, `rdata.grf`, and `data/` folder

**Option B: Using Full Client Package**

If you downloaded a full client installer:

1. Run the installer (e.g., `RO_Install_200120.exe`)
2. Follow the installation wizard
3. Install to: `C:\Ragnarok Online\` (or your preferred location)
4. After installation, replace the client executable with your preferred version if needed

### 2.4 Verify Client Installation

Check that you have the following structure:

```
C:\Ragnarok Online\
├── Ragexe.exe          (or 2018-06-20eRagexe.exe)
├── data.grf            (game data archive)
├── rdata.grf           (renewal data archive, if using Renewal)
├── data\               (additional game data folder)
│   ├── sprite\
│   ├── texture\
│   └── ...
└── ...
```

**Verification Steps**:

1. **Check executable exists**:
   ```powershell
   Test-Path "C:\Ragnarok Online\Ragexe.exe"
   # Should return: True
   ```

2. **Check file size** (approximate):
   ```powershell
   (Get-Item "C:\Ragnarok Online\Ragexe.exe").Length / 1MB
   # Should be around 3-8 MB depending on version
   ```

3. **Check data files exist**:
   ```powershell
   Test-Path "C:\Ragnarok Online\data.grf"
   # Should return: True
   ```

### 2.5 Client Version Compatibility Matrix

| Client Version | rAthena | WARP | P2P WebSocket | Windows 10/11 | Notes |
|----------------|---------|------|---------------|---------------|-------|
| 2018-06-20 | ✅ | ✅ | ✅ | ✅ | **Recommended** - Most stable |
| 2020-04-01 | ✅ | ✅ | ✅ | ✅ | Good alternative |
| 2019-05-30 | ✅ | ✅ | ✅ | ✅ | Balanced features |
| 2020-03-04 | ✅ | ✅ | ✅ | ✅ | Renewal recommended |
| 2021+ | ⚠️ | ⚠️ | ⚠️ | ✅ | May have compatibility issues |
| 2017 or older | ⚠️ | ⚠️ | ❌ | ✅ | Missing modern features |

**Legend**:
- ✅ Fully compatible
- ⚠️ May work with limitations
- ❌ Not compatible

### 2.6 Alternative Download Sources

If NEMO repository is unavailable, try these alternatives:

**1. rAthena Forum Resources**:
- Visit: https://rathena.org/board/
- Search for "client download" or "2018-06-20"
- Check pinned topics in Client-Side Support section

**2. Hercules Forum**:
- Visit: https://board.herc.ws/
- Similar client downloads available
- Community-verified sources

**3. Contact Server Administrator**:
- Your server admin may provide a pre-configured client
- This ensures compatibility with server settings
- May include custom patches and data files

### 2.7 Client Download Troubleshooting

**Problem: Download link is broken**

Solution:
1. Try alternative client versions (2020-04-01 or 2019-05-30)
2. Check NEMO clients page: http://nemo.herc.ws/clients/
3. Search for mirrors on rAthena/Hercules forums

**Problem: Downloaded file won't run**

Solution:
1. Verify file integrity (check file size matches expected)
2. Disable antivirus temporarily during download
3. Run as Administrator
4. Install Visual C++ Redistributables (see Step 1.2)

**Problem: Missing game data files**

Solution:
1. Download full client package instead of just executable
2. Contact server administrator for data files
3. Use data files from official RO client (if you have one)

**Problem: "This app can't run on your PC" error**

Solution:
1. Ensure you downloaded the correct architecture (32-bit client for RO)
2. Install DirectX 9.0c (see Step 1.3)
3. Try running in Windows 7 compatibility mode:
   ```powershell
   # Right-click Ragexe.exe → Properties → Compatibility
   # Check "Run this program in compatibility mode for: Windows 7"
   ```

---

## Step 3: Download WARP P2P Client

### 3.1 Download from GitHub

> **Note**: Pre-built releases are not yet available. Clone the repository to access pre-built executables in the `win32/` directory.

**Option A: Download Pre-built Release** *(Coming Soon)*

Once releases are available:
1. Go to the releases page: https://github.com/iskandarsulaili/WARP-p2p-client/releases
2. Download the latest release ZIP file
3. Extract to a folder (e.g., `C:\WARP-P2P\`)

**Option B: Clone Repository** *(Recommended - Use This Method)*

```powershell
# Install Git for Windows if not already installed
# Download from: https://git-scm.com/download/win

# Clone the repository
cd C:\
git clone https://github.com/iskandarsulaili/WARP-p2p-client.git
cd WARP-p2p-client
```

> **Good News**: The repository includes pre-built WARP executables in the `win32/` directory, so you don't need to build from source!

### 3.2 Verify Downloaded Files

Your WARP-p2p-client directory should contain:

```
WARP-p2p-client/
├── win32/
│   ├── WARP.exe              # Main WARP patcher GUI
│   ├── WARP_console.exe      # Console version
│   ├── Qt5*.dll              # Qt libraries
│   └── ...
├── Data/
│   ├── config.ini.example    # P2P configuration template
│   └── resourceinfo.ini.example
├── Patches/
│   ├── p2p_hosting.yml       # P2P hosting patches
│   └── ...
├── Scripts/
└── README.md
```

---

## Step 4: Configure P2P Settings

### 4.1 Create Configuration Files

```powershell
# Navigate to WARP directory
cd C:\WARP-p2p-client\Data

# Copy example configuration files
copy config.ini.example config.ini
copy resourceinfo.ini.example resourceinfo.ini
```

### 4.2 Edit P2P Configuration

Open `config.ini` in a text editor (Notepad++ recommended):

```ini
[P2P]
; Enable P2P hosting support
EnableP2P=1
PreferP2PHosting=1
MainServerFallback=1

; Coordinator connection
; For local testing: ws://localhost:8001/api/signaling/ws
; For production: wss://your-server-domain.com/api/signaling/ws
CoordinatorURL=ws://YOUR_SERVER_IP:8001/api/signaling/ws
CoordinatorPort=8001
ConnectionTimeout=5000
ReconnectDelay=1000
MaxRetries=3

; Host Requirements (adjust based on your system)
MinCPUCores=4
MinCPUFrequency=3000
MinMemoryMB=8192
MinNetworkSpeedMbps=100
MaxLatencyMS=100

; Security settings
EnableEncryption=1
KeyRotationInterval=3600

; Performance settings
CompressionThreshold=1024
UpdateInterval=50
MaxConnections=5
```

**Replace `YOUR_SERVER_IP`** with your Ubuntu server's IP address or domain name.

### 4.3 Edit Resource Configuration

Open `resourceinfo.ini` in a text editor:

```ini
[CONNECTION]
# Main server connection settings
ServerIP=YOUR_SERVER_IP
ServerPort=6900

[P2P_HOSTS]
# Primary P2P coordinator
0=ws://YOUR_SERVER_IP:8001/api/signaling/ws

# Backup coordinator (optional)
# 1=wss://backup-server.com/api/signaling/ws

[P2P_ZONES]
# Zones enabled for P2P hosting
prontera=1
geffen=1
payon=1
morocc=1
alberta=1
izlude=1
```

**Replace `YOUR_SERVER_IP`** with your server's IP address.

---

## Step 5: Using WARP to Patch Your Client

### 5.1 Launch WARP

```powershell
# Navigate to WARP directory
cd C:\WARP-p2p-client\win32

# Launch WARP GUI
.\WARP.exe
```

### 5.2 Load Your RO Client

1. Click **"Browse"** or **"Load Client"**
2. Navigate to your Ragnarok Online installation folder
3. Select your client executable (e.g., `Ragexe.exe`)
4. Click **"Open"**

### 5.3 Select P2P Patches

In the WARP interface:

1. Go to the **"Patches"** tab
2. Search for **"P2P"** or **"Hosting"**
3. Enable the following patches:
   - ✅ **Enable P2P Hosting**
   - ✅ **P2P WebSocket Integration**
   - ✅ **P2P Coordinator Connection**
4. Configure patch settings if prompted

### 5.4 Apply Patches

1. Click **"Apply Patches"** or **"Generate"**
2. Choose output location for patched client
3. Wait for patching to complete
4. Verify success message

> **Output**: WARP will create a patched version of your client (e.g., `Ragexe_patched.exe`)

---

## Step 6: Connecting to the Server

### 6.1 Copy Configuration Files

Copy the configured files to your RO client directory:

```powershell
# Copy config files to RO client directory
copy C:\WARP-p2p-client\Data\config.ini "C:\Program Files (x86)\Ragnarok Online\"
copy C:\WARP-p2p-client\Data\resourceinfo.ini "C:\Program Files (x86)\Ragnarok Online\"
```

> **Note**: Adjust the path to match your RO client installation directory.

### 6.2 Launch the Patched Client

```powershell
# Navigate to your RO client directory
cd "C:\Program Files (x86)\Ragnarok Online"

# Launch the patched client
.\Ragexe_patched.exe
```

### 6.3 Login to the Server

1. **Enter Login Credentials**:
   - Username: Your account username
   - Password: Your account password

2. **Select Character**:
   - Choose or create a character

3. **Enter Game World**:
   - The client will connect to the rAthena AI World server
   - P2P features will be automatically enabled for supported zones

### 6.4 Verify P2P Connection

**Check P2P Status**:
- Look for P2P status indicators in the game UI
- Check if you're connected to a P2P host or hosting yourself
- Verify coordinator connection in the client logs

**Test P2P Hosting** (if your system meets requirements):
1. Enter a P2P-enabled zone (e.g., Prontera)
2. Check if you become a host or connect to an existing host
3. Monitor network activity and performance

---

## Troubleshooting

### Client Won't Start

**Problem**: Patched client crashes on startup

**Solutions**:

1. **Verify Prerequisites**:
   ```powershell
   # Check .NET Framework
   Get-ItemProperty "HKLM:\SOFTWARE\Microsoft\NET Framework Setup\NDP\v4\Full"

   # Check Visual C++ Redistributables
   Get-ItemProperty "HKLM:\SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\*"
   ```

2. **Run as Administrator**:
   - Right-click `Ragexe_patched.exe`
   - Select "Run as administrator"

3. **Check Compatibility Mode**:
   - Right-click `Ragexe_patched.exe` → Properties
   - Go to "Compatibility" tab
   - Try "Windows 7" or "Windows 8" compatibility mode

4. **Disable Antivirus Temporarily**:
   - Some antivirus software may block patched executables
   - Add exception for your RO client directory

### Cannot Connect to Server

**Problem**: Client shows "Failed to connect to server"

**Solutions**:

1. **Verify Server IP**:
   - Check `resourceinfo.ini` has correct server IP
   - Ping the server: `ping YOUR_SERVER_IP`

2. **Check Firewall**:
   ```powershell
   # Allow RO client through Windows Firewall
   New-NetFirewallRule -DisplayName "Ragnarok Online" -Direction Outbound -Program "C:\Program Files (x86)\Ragnarok Online\Ragexe_patched.exe" -Action Allow
   ```

3. **Verify Server is Running**:
   - Ensure the Ubuntu server is running (see Ubuntu guide)
   - Check if rAthena servers are active
   - Verify ports 6900, 6121, 5121 are accessible

4. **Test Connection**:
   ```powershell
   # Test connection to login server
   Test-NetConnection -ComputerName YOUR_SERVER_IP -Port 6900
   ```

### P2P Features Not Working

**Problem**: P2P hosting is not enabled

**Solutions**:

1. **Verify P2P Patches Applied**:
   - Re-open client in WARP
   - Check if P2P patches are enabled
   - Re-apply patches if needed

2. **Check Configuration**:
   - Open `config.ini`
   - Verify `EnableP2P=1`
   - Verify `CoordinatorURL` is correct

3. **Test Coordinator Connection**:
   ```powershell
   # Test WebSocket connection (requires wscat or similar tool)
   # Install Node.js and wscat:
   npm install -g wscat

   # Test connection
   wscat -c ws://YOUR_SERVER_IP:8001/api/signaling/ws
   ```

4. **Check System Requirements**:
   - Verify your system meets minimum requirements for hosting
   - Check CPU, RAM, and network speed
   - Review `config.ini` host requirements settings

### High Latency or Lag

**Problem**: Game is laggy or has high latency

**Solutions**:

1. **Check Network Connection**:
   ```powershell
   # Test latency to server
   ping YOUR_SERVER_IP -n 100

   # Check network speed
   # Use online speed test: https://www.speedtest.net/
   ```

2. **Adjust P2P Settings**:
   - Edit `config.ini`
   - Increase `MaxLatencyMS` if needed
   - Reduce `UpdateInterval` for better performance

3. **Disable P2P Hosting**:
   - If your system doesn't meet requirements
   - Edit `config.ini`: `PreferP2PHosting=0`
   - This will make you a client only, not a host

4. **Close Background Applications**:
   - Close bandwidth-heavy applications
   - Disable Windows updates during gameplay
   - Close unnecessary browser tabs

### WARP Patcher Errors

**Problem**: WARP fails to patch the client

**Solutions**:

1. **Verify Client Executable**:
   - Ensure you're using a compatible RO client
   - Try different client versions (2018+ recommended)
   - Check if client is not corrupted

2. **Run WARP as Administrator**:
   - Right-click `WARP.exe`
   - Select "Run as administrator"

3. **Check WARP Logs**:
   - Look for error messages in WARP console
   - Check `Outputs/` directory for error logs

4. **Update WARP**:
   - Download latest version from GitHub
   - Replace old files with new ones

### Configuration File Issues

**Problem**: Configuration files not being read

**Solutions**:

1. **Verify File Location**:
   - Configuration files must be in the same directory as the client executable
   - Check file names: `config.ini` and `resourceinfo.ini` (not `.example`)

2. **Check File Encoding**:
   - Files should be in UTF-8 or ANSI encoding
   - Re-save files with correct encoding if needed

3. **Verify File Permissions**:
   - Ensure files are not read-only
   - Right-click → Properties → Uncheck "Read-only"

---

## Advanced Configuration

### Custom P2P Zones

To enable/disable P2P for specific zones, edit `resourceinfo.ini`:

```ini
[P2P_ZONES]
# Enable P2P for these zones
prontera=1
geffen=1
payon=1

# Disable P2P for these zones
guild_vs1=0
guild_vs2=0
pvp_y_room=0
```

### Performance Tuning

For better performance, adjust these settings in `config.ini`:

```ini
[P2P_NETWORK]
# Reduce update interval for smoother gameplay (lower = more CPU usage)
UpdateIntervalMS=50

# Increase max latency tolerance
MaxLatencyMS=200

# Adjust packet batch size
PacketBatchSize=32

[P2P_SECURITY]
# Disable compression for low-end systems
EnableCompression=0
CompressionThreshold=2048
```

### Debug Mode

To enable debug logging for troubleshooting:

```ini
[DEBUG]
EnableP2PDebug=1
LogLevel=DEBUG
LogToFile=1
LogFilePath=logs/p2p.log
```

Check the log file at `logs/p2p.log` for detailed information.

### Multiple Server Profiles

To connect to different servers, create multiple configuration files:

```powershell
# Create profiles
copy config.ini config_server1.ini
copy config.ini config_server2.ini

# Edit each profile with different server settings
# Use batch files to launch with specific profiles
```

**Launch script** (`launch_server1.bat`):

```batch
@echo off
copy /Y config_server1.ini config.ini
start Ragexe_patched.exe
```

---

## Performance Optimization

### System Optimization

1. **Disable Windows Visual Effects**:
   - Control Panel → System → Advanced → Performance Settings
   - Select "Adjust for best performance"

2. **Set High Performance Power Plan**:
   - Control Panel → Power Options
   - Select "High performance"

3. **Disable Background Apps**:
   - Settings → Privacy → Background apps
   - Disable unnecessary apps

### Network Optimization

1. **Use Wired Connection**:
   - Ethernet is more stable than Wi-Fi for gaming

2. **Close Bandwidth-Heavy Apps**:
   - Stop downloads, streaming, etc.

3. **Configure QoS** (Quality of Service):
   - Router settings → QoS
   - Prioritize gaming traffic

### Client Optimization

1. **Reduce Graphics Settings**:
   - Lower resolution
   - Disable effects
   - Reduce texture quality

2. **Limit Frame Rate**:
   - Cap FPS to reduce CPU/GPU usage

3. **Use Minimal UI**:
   - Hide unnecessary UI elements

---

## Security Considerations

### Firewall Configuration

**Allow Outbound Connections**:

```powershell
# Allow RO client outbound
New-NetFirewallRule -DisplayName "RO Client Out" -Direction Outbound -Program "C:\Program Files (x86)\Ragnarok Online\Ragexe_patched.exe" -Action Allow

# Allow P2P coordinator connection
New-NetFirewallRule -DisplayName "P2P Coordinator" -Direction Outbound -Protocol TCP -RemotePort 8001 -Action Allow
```

**Allow Inbound Connections** (for P2P hosting):

```powershell
# Allow WebRTC connections
New-NetFirewallRule -DisplayName "P2P WebRTC" -Direction Inbound -Protocol UDP -Action Allow
```

### Antivirus Exceptions

Add exceptions for:
- RO client directory: `C:\Program Files (x86)\Ragnarok Online\`
- WARP directory: `C:\WARP-p2p-client\`

### Safe Patching Practices

1. **Backup Original Client**:
   ```powershell
   copy Ragexe.exe Ragexe_backup.exe
   ```

2. **Verify Patches**:
   - Only use trusted patches
   - Review patch descriptions before applying

3. **Keep WARP Updated**:
   - Regularly check for updates
   - Download from official sources only

---

## Building WARP from Source (Advanced)

**⚠️ Note**: This section is for advanced users and developers who want to build WARP from source. Most users should use the pre-built executables in the `win32/` directory (see Step 3 above).

### When to Build from Source

Build from source if you:
- Want to contribute to WARP development
- Need to modify P2P networking features
- Want to debug issues in the WARP codebase
- Are developing custom patches or extensions

### Build Prerequisites

#### System Requirements

- **Operating System**: Windows 10 (64-bit) or Windows 11
- **RAM**: Minimum 8GB (16GB recommended for faster builds)
- **Disk Space**: At least 10GB free space for build tools and dependencies
- **Internet Connection**: Required for downloading dependencies

#### Required Knowledge

- Basic command-line usage
- Understanding of C++ compilation process (helpful but not required)

---

### Build Tools Installation

#### 1. Install Visual Studio 2019 or 2022

WARP requires Visual Studio with C++ development tools.

**Option A: Visual Studio Community (Free)**

1. Download Visual Studio Community from: https://visualstudio.microsoft.com/downloads/
2. Run the installer
3. Select **"Desktop development with C++"** workload
4. Under "Individual components", ensure these are selected:
   - MSVC v142 - VS 2019 C++ x64/x86 build tools (or v143 for VS 2022)
   - Windows 10 SDK (10.0.19041.0 or later)
   - C++ CMake tools for Windows
   - C++ ATL for latest build tools
5. Click "Install" (this may take 30-60 minutes)

**Option B: Build Tools for Visual Studio (Minimal)**

If you don't want the full Visual Studio IDE:

1. Download "Build Tools for Visual Studio" from the same page
2. Select the same components as above
3. Install

#### 2. Install Qt Framework

WARP uses Qt 5.15.x for its GUI and core functionality.

**Download Qt:**

1. Visit: https://www.qt.io/download-qt-installer
2. Download "Qt Online Installer for Windows"
3. Run the installer and create a Qt account (free)
4. Select Qt 5.15.2 (or latest 5.15.x version)
5. Under Qt 5.15.2, select:
   - **MSVC 2019 64-bit** (or MSVC 2022 64-bit)
   - **Qt WebEngine**
   - **Qt Network Authorization**
6. Install location: `C:\Qt\5.15.2` (default is fine)

**Add Qt to PATH:**

```cmd
setx PATH "%PATH%;C:\Qt\5.15.2\msvc2019_64\bin"
```

#### 3. Install CMake

CMake is required for building the P2P components.

1. Download CMake from: https://cmake.org/download/
2. Choose "Windows x64 Installer"
3. During installation, select **"Add CMake to system PATH for all users"**
4. Verify installation:
   ```cmd
   cmake --version
   ```

#### 4. Install Git for Windows

1. Download from: https://git-scm.com/download/win
2. Install with default options
3. Verify:
   ```cmd
   git --version
   ```

---

### Build Dependencies

#### 1. Clone the Repository

```cmd
cd C:\
git clone https://github.com/iskandarsulaili/WARP-p2p-client.git
cd WARP-p2p-client
```

#### 2. Install P2P Dependencies

The P2P features require additional libraries:

**WebSocket++ (Header-only)**

```cmd
cd C:\
git clone https://github.com/zaphoyd/websocketpp.git
```

**Asio (Header-only)**

```cmd
cd C:\
git clone https://github.com/chriskohlhoff/asio.git
```

**nlohmann/json (Header-only)**

```cmd
cd C:\
git clone https://github.com/nlohmann/json.git
```

**OpenSSL**

1. Download pre-built OpenSSL from: https://slproweb.com/products/Win32OpenSSL.html
2. Choose "Win64 OpenSSL v3.x.x" (not the "Light" version)
3. Install to `C:\OpenSSL-Win64`
4. Add to PATH:
   ```cmd
   setx PATH "%PATH%;C:\OpenSSL-Win64\bin"
   ```

---

### Building WARP P2P Client

#### Method 1: Using Pre-built Executables (Recommended)

The repository includes pre-built executables in the `win32/` directory:

```cmd
cd C:\WARP-p2p-client\win32
dir
```

You should see:
- `WARP.exe` - Main GUI application
- `WARP_console.exe` - Console version
- `WARP_bench.exe` - Benchmarking tool
- Various Qt DLL files

**Skip to verification if using pre-built executables.**

#### Method 2: Building from Source (For Developers)

**Note**: The original WARP patcher is primarily JavaScript-based and doesn't require compilation. However, the P2P extensions require building C++ components.

**Step 1: Build P2P Network Components**

```cmd
cd C:\WARP-p2p-client
mkdir build
cd build
```

**Step 2: Configure with CMake**

```cmd
cmake .. -G "Visual Studio 16 2019" -A x64 ^
  -DENABLE_P2P=ON ^
  -DQt5_DIR=C:\Qt\5.15.2\msvc2019_64\lib\cmake\Qt5 ^
  -DWEBSOCKETPP_INCLUDE_DIR=C:\websocketpp ^
  -DJSON_INCLUDE_DIR=C:\json\include ^
  -DASIO_INCLUDE_DIR=C:\asio\asio\include ^
  -DOPENSSL_ROOT_DIR=C:\OpenSSL-Win64
```

**For Visual Studio 2022:**

```cmd
cmake .. -G "Visual Studio 17 2022" -A x64 ^
  -DENABLE_P2P=ON ^
  -DQt5_DIR=C:\Qt\5.15.2\msvc2022_64\lib\cmake\Qt5 ^
  -DWEBSOCKETPP_INCLUDE_DIR=C:\websocketpp ^
  -DJSON_INCLUDE_DIR=C:\json\include ^
  -DASIO_INCLUDE_DIR=C:\asio\asio\include ^
  -DOPENSSL_ROOT_DIR=C:\OpenSSL-Win64
```

**Step 3: Build the Project**

```cmd
cmake --build . --config Release
```

This will compile the P2P network components. The build process may take 5-15 minutes depending on your system.

**Step 4: Copy Built Files**

After successful build:

```cmd
copy Release\*.dll ..\win32\
copy Release\*.exe ..\win32\
```

---

### Build Troubleshooting

#### Common Build Errors

**Error: "Qt5 not found"**

Solution:
- Verify Qt installation path
- Ensure you installed the correct MSVC version (2019 or 2022)
- Update the `-DQt5_DIR` path in CMake command

**Error: "OpenSSL not found"**

Solution:
```cmd
setx OPENSSL_ROOT_DIR "C:\OpenSSL-Win64"
setx OPENSSL_INCLUDE_DIR "C:\OpenSSL-Win64\include"
setx OPENSSL_LIBRARIES "C:\OpenSSL-Win64\lib"
```

Restart your command prompt and try again.

**Error: "WebSocket++ headers not found"**

Solution:
- Verify the clone location: `C:\websocketpp`
- Check that `C:\websocketpp\websocketpp\config\asio_client.hpp` exists
- Update the `-DWEBSOCKETPP_INCLUDE_DIR` path in CMake command

**Error: "MSVC compiler not found"**

Solution:
- Open "Developer Command Prompt for VS 2019" (or VS 2022) from Start Menu
- Run the CMake commands from this special command prompt
- Alternatively, run this before CMake:
  ```cmd
  "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
  ```

**Error: "LNK1104: cannot open file 'Qt5Core.lib'"**

Solution:
- Ensure Qt bin directory is in PATH
- Try building from Qt Creator instead:
  1. Open Qt Creator
  2. File → Open File or Project
  3. Select `CMakeLists.txt`
  4. Configure with MSVC kit
  5. Build → Build All

#### Build Performance Issues

**Slow compilation:**
- Use `/MP` flag for parallel compilation:
  ```cmd
  cmake --build . --config Release -- /m
  ```
- Close other applications to free up RAM
- Use SSD for build directory

**Out of memory errors:**
- Reduce parallel jobs:
  ```cmd
  cmake --build . --config Release -- /m:2
  ```

#### Runtime Errors

**"VCRUNTIME140.dll not found"**

Solution:
- Install Visual C++ Redistributable:
  - Download from: https://aka.ms/vs/17/release/vc_redist.x64.exe
  - Run and install

**"Qt5Core.dll not found"**

Solution:
- Copy Qt DLLs to the same directory as WARP.exe:
  ```cmd
  cd C:\WARP-p2p-client\win32
  windeployqt WARP.exe
  ```

**"The application failed to start because its side-by-side configuration is incorrect"**

Solution:
- Rebuild with matching Visual Studio version
- Ensure all dependencies are 64-bit (not 32-bit)

---

### Build Verification

#### 1. Verify Pre-built Executables

```cmd
cd C:\WARP-p2p-client\win32
WARP.exe --version
```

Expected output:
```
WARP P2P Client v1.0.0
```

#### 2. Test WARP GUI

```cmd
WARP.exe
```

The WARP GUI should open. You should see:
- Main window with patch selection interface
- P2P settings tab (if P2P support is enabled)
- No error messages in the console

#### 3. Verify P2P Components

```cmd
WARP_console.exe --test-p2p
```

Expected output:
```
P2P WebSocket support: Enabled
P2P Coordinator: Not connected (expected)
```

#### 4. Check Dependencies

```cmd
cd C:\WARP-p2p-client\win32
dir *.dll
```

Required DLLs:
- `Qt5Core.dll`
- `Qt5Gui.dll`
- `Qt5Network.dll`
- `Qt5Qml.dll`
- `Qt5Quick.dll`
- `YAML.dll`
- `GATE.dll`
- `libEGL.dll`
- `libGLESv2.dll`
- `msvcp140.dll`
- `vcruntime140.dll`

---

### Build Configuration Options

#### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `ENABLE_P2P` | `ON` | Enable P2P hosting support |
| `CMAKE_BUILD_TYPE` | `Release` | Build type (Release/Debug) |
| `Qt5_DIR` | Auto-detect | Path to Qt5 CMake files |
| `WEBSOCKETPP_INCLUDE_DIR` | Auto-detect | WebSocket++ headers path |
| `JSON_INCLUDE_DIR` | Auto-detect | nlohmann/json headers path |
| `ASIO_INCLUDE_DIR` | Auto-detect | Asio headers path |
| `OPENSSL_ROOT_DIR` | Auto-detect | OpenSSL installation path |

#### Debug Build

For development and debugging:

```cmd
cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

Debug builds include:
- Symbol information for debugging
- Additional runtime checks
- Verbose logging
- Larger executable size

#### Clean Build

To start fresh:

```cmd
cd C:\WARP-p2p-client
rmdir /s /q build
mkdir build
cd build
```

Then repeat the CMake configuration and build steps.

---

### Build Resources

**Official Documentation:**
- **WARP Wiki**: https://github.com/Neo-Mind/WARP/wiki
- **Qt Documentation**: https://doc.qt.io/qt-5/
- **CMake Documentation**: https://cmake.org/documentation/

**Development Tools:**
- **Qt Creator**: Recommended IDE for Qt development
  - Download: https://www.qt.io/download-qt-installer
  - Includes visual designer and debugger
- **Visual Studio Code**: Alternative lightweight editor
  - Install C++ extension
  - Install CMake Tools extension

---

## Next Steps

After successful setup:

1. **Join the Game**:
   - Create your character
   - Explore the AI-driven world
   - Interact with AI NPCs

2. **Test P2P Features**:
   - Visit P2P-enabled zones
   - Check if you become a host
   - Monitor performance

3. **Provide Feedback**:
   - Report bugs or issues
   - Share your experience
   - Contribute to improvements

4. **Explore Advanced Features**:
   - Try different configurations
   - Experiment with performance settings
   - Join community discussions

---

## Additional Resources

- **WARP Documentation**: `WARP-p2p-client/README.md`
- **P2P Integration Guide**: `WARP-p2p-client/IMPLEMENTATION_GUIDE.md`
- **Server Setup Guide**: `rathena-AI-world/UBUNTU_SERVER_DEPLOYMENT_GUIDE.md`
- **P2P Coordinator API**: `rathena-AI-world/p2p-coordinator/docs/API.md`

---

## Support

For issues or questions:
- Check troubleshooting section above
- Review WARP logs in `Outputs/` directory
- Check client logs in RO directory
- Consult server administrator for server-side issues

---

**Congratulations!** 🎉 You have successfully set up the WARP P2P client on Windows and connected to the rAthena AI World server!
