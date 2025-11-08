# P2P Network DLL - Build Guide

**Last Updated**: 2025-11-08  
**Platform**: ⚠️ **WINDOWS 10/11 REQUIRED** (NOT Linux)  
**Compiler**: MSVC 2022 (Visual Studio 2022 BuildTools)  
**CMake**: 3.30.1+  
**Package Manager**: vcpkg  
**WebRTC Library**: libdatachannel 0.23.2

---

## Table of Contents

1. [Critical Information](#critical-information)
2. [Prerequisites](#prerequisites)
3. [vcpkg Setup on D:/ Drive](#vcpkg-setup-on-d-drive)
4. [Installing Dependencies](#installing-dependencies)
5. [Build Steps](#build-steps)
6. [Verification](#verification)
7. [Troubleshooting](#troubleshooting)
8. [Running Tests](#running-tests)
9. [Installation](#installation)

---

## ⚠️ CRITICAL: Windows Build Required

**The P2P DLL MUST be built on Windows**, not Linux/Ubuntu, because:

- Ragnarok Online client is a Windows application
- DLLs are Windows-specific binaries (.dll files)
- NEMO patcher patches Windows executables
- Project requires MSVC compiler (Windows-only)

**Linux builds produce .so files** which will NOT work with the Windows RO client.

---

## Prerequisites

### Required Software (Windows)

#### 1. Visual Studio 2022 BuildTools or Community Edition

**Download:**

- BuildTools: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
- Community: https://visualstudio.microsoft.com/downloads/

**Required Components:**

- MSVC v143 - VS 2022 C++ x64/x86 build tools (Latest)
- Windows 10/11 SDK (10.0.26100.0 or later)
- C++ CMake tools for Windows

**Compiler Version:** cl.exe 14.44.35207 or later

**Verification:**

```powershell
# Open "x64 Native Tools Command Prompt for VS 2022"
cl
# Should show: Microsoft (R) C/C++ Optimizing Compiler Version 19.44.35207 for x64
```

#### 2. CMake 3.30.1+

**Download:** https://cmake.org/download/

**Recommended Installation Path:** `D:\Program Files\CMake\`

**Installation:**

- Download Windows x64 Installer
- During installation, select "Add CMake to system PATH"
- Install to `D:\Program Files\CMake\` (or default location)

**Verification:**

```powershell
cmake --version
# Should show: cmake version 3.30.1 or higher
```

#### 3. Git for Windows

**Download:** https://git-scm.com/download/win

**Installation:**

- Use default settings
- Select "Git from the command line and also from 3rd-party software"

**Verification:**

```powershell
git --version
# Should show: git version 2.x.x
```

---

## vcpkg Setup on D:/ Drive

**⚠️ IMPORTANT:** Install vcpkg on the **D:/ drive** to avoid C:/ drive space issues.

vcpkg will download and build many packages (especially Boost), which can consume **10+ GB** of disk space.

### Step 1: Clone vcpkg to D:/ Drive

```powershell
# Clone vcpkg to D:/ drive
git clone https://github.com/Microsoft/vcpkg.git D:\vcpkg

# Navigate to vcpkg directory
cd D:\vcpkg
```

### Step 2: Bootstrap vcpkg

```powershell
# Run bootstrap script (this compiles vcpkg itself)
.\bootstrap-vcpkg.bat
```

**Expected Output:**

```
Downloading vcpkg-glibc...
Building vcpkg.exe...
vcpkg package management program version 2024-XX-XX
```

### Step 3: Integrate with Visual Studio

```powershell
# Integrate vcpkg with Visual Studio (makes packages available globally)
.\vcpkg integrate install
```

**Expected Output:**

```
Applied user-wide integration for this vcpkg root.
CMake projects should use: "-DCMAKE_TOOLCHAIN_FILE=D:/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

### Step 4: Verify Installation

```powershell
# Check vcpkg version
.\vcpkg version

# List installed packages (should be empty initially)
.\vcpkg list
```

---

## Installing Dependencies

### Required Packages

The P2P-DLL requires the following packages:

| Package        | Version | Purpose                 | Install Time |
| -------------- | ------- | ----------------------- | ------------ |
| nlohmann-json  | 3.12.0  | JSON parsing            | ~2 min       |
| spdlog         | 1.16.0  | Logging framework       | ~3 min       |
| openssl        | 3.6.0   | SSL/TLS encryption      | ~10 min      |
| cpp-httplib    | 0.27.0  | HTTP client             | ~2 min       |
| gtest          | 1.17.0  | Testing framework       | ~5 min       |
| detours        | 4.0.1   | DLL injection           | ~1 min       |
| boost          | 1.89.0  | Boost.Beast (WebSocket) | **~90 min**  |
| libdatachannel | 0.23.2  | WebRTC data channels    | ~10 min      |

**Total Installation Time:** ~2 hours (mostly Boost compilation)

### Installation Commands

```powershell
# Navigate to vcpkg directory
cd D:\vcpkg

# Install all dependencies (this will take ~2 hours)
.\vcpkg install nlohmann-json:x64-windows
.\vcpkg install spdlog:x64-windows
.\vcpkg install openssl:x64-windows
.\vcpkg install cpp-httplib:x64-windows
.\vcpkg install gtest:x64-windows
.\vcpkg install detours:x64-windows

# Install Boost components (this takes the longest - ~90 minutes)
.\vcpkg install boost:x64-windows

# Install libdatachannel for WebRTC
.\vcpkg install libdatachannel:x64-windows
```

**💡 Tip:** You can install all packages in one command:

```powershell
.\vcpkg install nlohmann-json spdlog openssl cpp-httplib gtest detours boost libdatachannel --triplet x64-windows
```

### Verify Package Installation

```powershell
# List all installed packages
.\vcpkg list | Select-String "nlohmann-json|spdlog|openssl|httplib|gtest|detours|boost|libdatachannel"
```

**Expected Output (partial):**

```
boost:x64-windows                                  1.89.0
cpp-httplib[brotli,core]:x64-windows               0.27.0
detours:x64-windows                                4.0.1#8
gtest:x64-windows                                  1.17.0#2
libdatachannel[core,ws]:x64-windows                0.23.2
nlohmann-json:x64-windows                          3.12.0#1
openssl:x64-windows                                3.6.0#3
spdlog:x64-windows                                 1.16.0
```

---

## Build Steps

### Step 1: Open Developer Command Prompt

You **must** use the Visual Studio Developer Command Prompt to ensure the MSVC compiler is available.

**Option A: Use Start Menu**

1. Press Windows key
2. Search for "x64 Native Tools Command Prompt for VS 2022"
3. Run as Administrator (recommended)

**Option B: Use PowerShell**

```powershell
# Launch VS Developer Shell in PowerShell
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64
```

### Step 2: Navigate to Project Directory

```powershell
# Navigate to your P2P-DLL project directory
cd "C:\Users\YourUsername\Desktop\FE YSS\WARP-p2p-client\P2P-DLL"
```

### Step 3: Configure with CMake

```powershell
# Create build directory
mkdir build -Force
cd build

# Configure CMake with vcpkg toolchain
& "D:\Program Files\CMake\bin\cmake.exe" `
  -S ".." `
  -B "." `
  -DCMAKE_TOOLCHAIN_FILE=D:/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="D:/vcpkg/installed/x64-windows"
```

**Expected Output:**

```
-- Running vcpkg install
-- Running vcpkg install - done
-- Selecting Windows SDK version 10.0.26100.0 to target Windows 10.0.22631.
-- The CXX compiler identification is MSVC 19.44.35207
-- Found nlohmann_json
-- Found spdlog
-- Found OpenSSL
-- Found boost_system
-- Found boost_asio
-- Found httplib
-- Found LibDataChannel
-- Configuring done (10.5s)
-- Generating done (0.2s)
-- Build files have been written to: C:/Users/.../P2P-DLL/build
```

### Step 4: Build the DLL

```powershell
# Build in Release mode
& "D:\Program Files\CMake\bin\cmake.exe" --build "." --config Release
```

**Expected Output:**

```
MSBuild version 17.14.23+b0019275e for .NET Framework

  1>Checking Build System
  Building Custom Rule .../P2P-DLL/CMakeLists.txt
  NetworkManager.cpp
  ConfigManager.cpp
  SignalingClient.cpp
  HttpClient.cpp
  PacketRouter.cpp
  WebRTCManager.cpp
  WebRTCPeerConnection.cpp
  SecurityManager.cpp
  AuthManager.cpp
  Logger.cpp
  DllMain.cpp
  Generating Code...
  p2p_network.vcxproj -> .../P2P-DLL/build/bin/Release/p2p_network.dll
  p2p_tests.vcxproj -> .../P2P-DLL/build/bin/Release/p2p_tests.exe
```

**Build Time:** ~5-10 minutes (first build), ~1-2 minutes (incremental builds)

---

## Verification

### Verify Build Output

```powershell
# Check if DLL was created
Get-Item "bin\Release\p2p_network.dll" | Format-List Name, Length, LastWriteTime
```

**Expected Output:**

```
Name          : p2p_network.dll
Length        : 515584  (approximately 503 KB)
LastWriteTime : 08-Nov-25 8:55:30 AM
```

### Verify DLL Dependencies

```powershell
# Use dumpbin to check DLL dependencies
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\dumpbin.exe" /DEPENDENTS "bin\Release\p2p_network.dll"
```

**Expected Dependencies:**

```
spdlog.dll
libssl-3-x64.dll
libcrypto-3-x64.dll
WS2_32.dll
brotlidec.dll
KERNEL32.dll
MSVCP140.dll
VCRUNTIME140.dll
```

### Verify Exported Functions

```powershell
# Check exported P2P functions
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\dumpbin.exe" /EXPORTS "bin\Release\p2p_network.dll" | Select-String "P2P_"
```

**Expected Exports:**

```
P2P_GetLastError
P2P_GetStatus
P2P_Initialize
P2P_IsActive
P2P_IsEnabled
P2P_Shutdown
P2P_Start
```

### Check Required DLLs in Build Directory

```powershell
# List all DLLs in Release directory
Get-ChildItem "bin\Release" -Filter "*.dll" | Select-Object Name, Length | Sort-Object Name
```

**Expected Files:**

```
Name                 Length
----                 ------
brotlicommon.dll     137728
brotlidec.dll         52224
fmt.dll              120832
libcrypto-3-x64.dll 5327872
libssl-3-x64.dll     871424
p2p_network.dll      515584
spdlog.dll           285696
```

---

## Troubleshooting

### Common Build Errors

#### Error: "Could not find a package configuration file provided by LibDataChannel"

**Cause:** libdatachannel not installed

**Solution:**

```powershell
cd D:\vcpkg
.\vcpkg install libdatachannel:x64-windows
```

#### Error: "Could not find a package configuration file provided by boost_system"

**Cause:** Boost not installed or incomplete installation

**Solution:**

```powershell
cd D:\vcpkg
.\vcpkg install boost:x64-windows
# This will take ~90 minutes
```

#### Error: "rtc/rtc.hpp: No such file or directory"

**Cause:** libdatachannel headers not found

**Solution:**

```powershell
# Reinstall libdatachannel
cd D:\vcpkg
.\vcpkg remove libdatachannel:x64-windows
.\vcpkg install libdatachannel:x64-windows
```

#### Error: "CMAKE_TOOLCHAIN_FILE not set"

**Cause:** vcpkg toolchain file not specified

**Solution:**

```powershell
# Always specify the toolchain file
cmake .. -DCMAKE_TOOLCHAIN_FILE=D:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

#### Error: "MSVC compiler not found"

**Cause:** Not using Visual Studio Developer Command Prompt

**Solution:**

```powershell
# Open "x64 Native Tools Command Prompt for VS 2022" from Start Menu
# Or run this in PowerShell:
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64
```

#### Error: "LNK1104: cannot open file 'spdlog.lib'"

**Cause:** spdlog not installed or vcpkg integration not working

**Solution:**

```powershell
# Reinstall spdlog
cd D:\vcpkg
.\vcpkg install spdlog:x64-windows

# Re-integrate vcpkg
.\vcpkg integrate install
```

#### Error: "C2664: cannot convert argument 1 from 'std::byte _' to 'const uint8_t _'"

**Cause:** Type mismatch between libdatachannel and our API (already fixed in current code)

**Solution:** This error should not occur in the latest code. If it does, ensure you have the latest version from the repository.

### Performance Issues

#### Build is Very Slow

**Cause:** Boost compilation is CPU-intensive

**Solutions:**

- Use `/MP` flag for multi-processor compilation (already enabled in CMakeLists.txt)
- Close other applications during build
- First build takes ~90 minutes for Boost, subsequent builds are much faster

#### vcpkg Downloads are Slow

**Cause:** Network speed or vcpkg mirror issues

**Solutions:**

- Use a faster internet connection
- Wait patiently - downloads are cached and only happen once
- Consider using vcpkg binary caching

---

## Running Tests

### Build Tests

```powershell
# Navigate to build directory
cd build

# Build tests
& "D:\Program Files\CMake\bin\cmake.exe" --build "." --config Release --target p2p_tests
```

### Run All Tests

```powershell
# Run tests using CTest
ctest -C Release --output-on-failure
```

**Expected Output:**

```
Test project C:/Users/.../P2P-DLL/build
    Start 1: ConfigManagerTest
1/2 Test #1: ConfigManagerTest ................   Passed    0.05 sec
    Start 2: HttpClientTest
2/2 Test #2: HttpClientTest ...................   Passed    0.03 sec

100% tests passed, 0 tests failed out of 2
```

### Run Specific Test

```powershell
# Run only ConfigManagerTest
ctest -C Release -R ConfigManagerTest -V
```

### Run Tests Directly

```powershell
# Run test executable directly
.\bin\Release\p2p_tests.exe
```

---

## Installation

### Copy DLL to Ragnarok Online Directory

```powershell
# Assuming RO is installed at C:\Program Files (x86)\Gravity\RO\

# Copy main DLL
copy bin\Release\p2p_network.dll "C:\Program Files (x86)\Gravity\RO\"

# Copy all required dependency DLLs
copy bin\Release\spdlog.dll "C:\Program Files (x86)\Gravity\RO\"
copy bin\Release\libssl-3-x64.dll "C:\Program Files (x86)\Gravity\RO\"
copy bin\Release\libcrypto-3-x64.dll "C:\Program Files (x86)\Gravity\RO\"
copy bin\Release\brotlidec.dll "C:\Program Files (x86)\Gravity\RO\"
copy bin\Release\brotlicommon.dll "C:\Program Files (x86)\Gravity\RO\"
copy bin\Release\fmt.dll "C:\Program Files (x86)\Gravity\RO\"

# Copy configuration file
copy ..\config\p2p_config.json "C:\Program Files (x86)\Gravity\RO\"

# Create logs directory
mkdir "C:\Program Files (x86)\Gravity\RO\logs" -Force
```

### Apply NEMO Patch

1. Open NEMO patcher
2. Load your RO client executable (e.g., `Ragnarok.exe`)
3. Find "Load P2P Network DLL" patch (located in `Patches/LoadP2PDLL.qs`)
4. Check the box to enable the patch
5. Click "Apply Patches"
6. Save the patched executable

**See DEPLOYMENT_GUIDE.md for detailed NEMO integration instructions.**

---

## Next Steps

After successful build:

1. ✅ **Verify DLL loads correctly** - Check Windows Event Viewer for DLL load events
2. ✅ **Configure p2p_config.json** - Set coordinator URL, STUN/TURN servers
3. ✅ **Apply NEMO patch** - Integrate DLL with RO client
4. ✅ **Test with P2P disabled** - Verify graceful fallback
5. ✅ **Test WebRTC connections** - Create test peer connections
6. ✅ **Deploy coordinator server** - Set up signaling server

**See also:**

- **WEBRTC_GUIDE.md** - WebRTC implementation details and usage
- **API_REFERENCE.md** - Complete API documentation
- **DEPLOYMENT_GUIDE.md** - Production deployment instructions
- **INTEGRATION_TEST_PLAN.md** - Integration testing procedures

---

## Build Summary

### Successful Build Checklist

- [ ] Visual Studio 2022 BuildTools installed
- [ ] CMake 3.30.1+ installed
- [ ] vcpkg installed on D:/ drive
- [ ] All dependencies installed via vcpkg
- [ ] CMake configuration successful
- [ ] Build completed without errors
- [ ] p2p_network.dll created (~503 KB)
- [ ] All dependency DLLs present in bin/Release/
- [ ] Exported functions verified (7 P2P\_\* functions)
- [ ] Tests pass successfully

### Build Artifacts

After a successful build, you should have:

```
P2P-DLL/build/
├── bin/
│   └── Release/
│       ├── p2p_network.dll      (503 KB - Main DLL)
│       ├── p2p_tests.exe        (Test executable)
│       ├── spdlog.dll           (285 KB)
│       ├── libssl-3-x64.dll     (871 KB)
│       ├── libcrypto-3-x64.dll  (5.3 MB)
│       ├── brotlidec.dll        (52 KB)
│       ├── brotlicommon.dll     (137 KB)
│       └── fmt.dll              (120 KB)
└── lib/
    └── Release/
        └── p2p_network.lib      (Import library)
```

---

## Additional Resources

- **CMake Documentation:** https://cmake.org/documentation/
- **vcpkg Documentation:** https://vcpkg.io/en/docs/README.html
- **libdatachannel GitHub:** https://github.com/paullouisageneau/libdatachannel
- **Boost Documentation:** https://www.boost.org/doc/
- **MSVC Compiler:** https://docs.microsoft.com/en-us/cpp/

---

**Build Guide Version:** 2.0
**Last Updated:** November 8, 2025
**Maintained by:** rAthena AI World Development Team
