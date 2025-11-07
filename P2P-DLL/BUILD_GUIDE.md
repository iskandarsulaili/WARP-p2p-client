# P2P Network DLL - Build Guide

**Last Updated**: 2025-11-07
**Platform**: ⚠️ **WINDOWS 10/11 REQUIRED** (NOT Linux)
**Compiler**: MSVC 2022 (Visual Studio 2022)
**CMake**: 3.25+
**Package Manager**: vcpkg

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

1. **Visual Studio 2022 Community Edition**
   - Download: https://visualstudio.microsoft.com/downloads/
   - Select "Desktop development with C++" workload
   - Includes MSVC compiler and Windows SDK

2. **CMake 3.25+**
   - Download: https://cmake.org/download/
   - Add to PATH during installation

3. **Git for Windows**
   - Download: https://git-scm.com/download/win

4. **vcpkg Package Manager**
   ```powershell
   git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
   cd C:\vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install
   ```

### Required Dependencies (via vcpkg)

Install all dependencies using vcpkg:

```powershell
cd C:\vcpkg
.\vcpkg install nlohmann-json:x64-windows
.\vcpkg install spdlog:x64-windows
.\vcpkg install openssl:x64-windows
.\vcpkg install websocketpp:x64-windows
.\vcpkg install cpp-httplib:x64-windows
.\vcpkg install gtest:x64-windows
```

**Note**: This will take 10-30 minutes depending on your internet speed and CPU.

### Package Verification

Verify vcpkg packages are installed:

```powershell
cd C:\vcpkg
.\vcpkg list | Select-String "nlohmann-json|spdlog|openssl|websocketpp|httplib|gtest"
```

**Expected Output**:
```
cpp-httplib:x64-windows
gtest:x64-windows
nlohmann-json:x64-windows
openssl:x64-windows
spdlog:x64-windows
websocketpp:x64-windows
```

---

## Build Steps (Windows)

### 1. Transfer Code to Windows

If you're developing on Linux/Ubuntu, transfer the P2P-DLL directory to Windows:

```bash
# On Ubuntu/Linux
cd /home/lot399/ai-mmorpg-world
tar -czf P2P-DLL.tar.gz WARP-p2p-client/P2P-DLL/

# Transfer P2P-DLL.tar.gz to Windows via USB, network share, etc.
```

On Windows, extract to a convenient location (e.g., `C:\Projects\P2P-DLL\`)

### 2. Open Developer Command Prompt

- Open "x64 Native Tools Command Prompt for VS 2022"
- Or open PowerShell and run:
  ```powershell
  & "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1"
  ```

### 3. Configure with CMake

```powershell
cd C:\Projects\P2P-DLL
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
```

**Expected Output**:
```
-- The CXX compiler identification is MSVC 19.38.33134.0
-- Found nlohmann_json
-- Found spdlog
-- Found OpenSSL
-- Found websocketpp
-- Found httplib
-- Configuring done
-- Generating done
```

### 4. Build the DLL

```powershell
cmake --build . --config Release
```

**Expected Output**:
```
[ 10%] Building CXX object CMakeFiles/p2p_network.dir/src/DllMain.cpp.obj
[ 20%] Building CXX object CMakeFiles/p2p_network.dir/src/core/ConfigManager.cpp.obj
[ 30%] Building CXX object CMakeFiles/p2p_network.dir/src/core/NetworkManager.cpp.obj
...
[100%] Linking CXX shared library bin\Release\p2p_network.dll
[100%] Built target p2p_network
```

### 5. Verify Build Output

```powershell
dir build\bin\Release\
```

**Expected Files**:
- `p2p_network.dll` (the main DLL)
- `p2p_network.lib` (import library)
- `p2p_network.pdb` (debug symbols)
- Size: ~500KB - 2MB (depending on dependencies)

---

## Build Troubleshooting (Windows)

### Error: "Could not find a package configuration file provided by spdlog"

**Solution**: Install via vcpkg
```powershell
cd C:\vcpkg
.\vcpkg install spdlog:x64-windows
```

### Error: "Could not find a package configuration file provided by httplib"

**Solution**: Install via vcpkg
```powershell
cd C:\vcpkg
.\vcpkg install cpp-httplib:x64-windows
```

### Error: "nlohmann/json.hpp: No such file or directory"

**Solution**: Install via vcpkg
```powershell
cd C:\vcpkg
.\vcpkg install nlohmann-json:x64-windows
```

### Error: "websocketpp/config/asio_no_tls.hpp: No such file or directory"

**Solution**: Install via vcpkg
```powershell
cd C:\vcpkg
.\vcpkg install websocketpp:x64-windows
```

### Error: "openssl/evp.h: No such file or directory"

**Solution**: Install via vcpkg
```powershell
cd C:\vcpkg
.\vcpkg install openssl:x64-windows
```

### Error: "CMake Error: CMAKE_TOOLCHAIN_FILE not set"

**Solution**: Specify vcpkg toolchain file
```powershell
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

### Error: "MSVC compiler not found"

**Solution**: Open "x64 Native Tools Command Prompt for VS 2022" or run:
```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1"
```

---

## Running Tests (Windows)

### Build Tests

```powershell
cd build
cmake --build . --config Release --target tests
```

### Run All Tests

```powershell
ctest -C Release --output-on-failure
```

### Run Specific Test

```powershell
ctest -C Release -R ConfigManagerTest -V
```

---

## Installation (Windows)

### Copy DLL to Ragnarok Online Directory

```powershell
# Assuming RO is installed at C:\Program Files (x86)\Gravity\RO\

# Copy DLL
copy build\bin\Release\p2p_network.dll "C:\Program Files (x86)\Gravity\RO\"

# Copy configuration
copy config\p2p_config.json "C:\Program Files (x86)\Gravity\RO\"

# Create logs directory
mkdir "C:\Program Files (x86)\Gravity\RO\logs"
```

### Apply NEMO Patch

1. Open NEMO patcher
2. Load your RO client executable (e.g., `Ragnarok.exe`)
3. Find "Load P2P Network DLL" under "Network" category
4. Check the box to enable the patch
5. Click "Apply Patches"
6. Save the patched executable

---

## Next Steps

After successful build:

1. ✅ Verify DLL loads correctly
2. ✅ Apply NEMO patch to RO client
3. ✅ Test with P2P disabled (graceful fallback)
4. ✅ Run integration tests (see INTEGRATION_TEST_PLAN.md)
5. ⏳ Fix critical issues (see CRITICAL_ISSUES_FIX_PLAN.md)

---

## Known Issues

1. **WebRTC is stub implementation** - Will not establish real P2P connections
2. **JWT authentication missing** - Coordinator endpoint not implemented
3. **Encryption is stub** - Packets not actually encrypted

See `CRITICAL_ISSUES_FIX_PLAN.md` for complete list and fix plan.

