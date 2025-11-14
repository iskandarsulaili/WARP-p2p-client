# P2P Network DLL Injector Guide

## Overview

This guide covers the alternative DLL injection method for loading `p2p_network.dll` when the WARP CustomDLL patch is not working correctly.

## Quick Start

### 1. Compile the Injector

**Using PowerShell:**
```powershell
.\build_injector.ps1
```

**Using Visual Studio Command Prompt:**
```batch
.\build_injector.bat
```

### 2. Run the Injector

```powershell
.\p2p_injector.exe
```

The injector will:
- Check for admin privileges and request elevation if needed
- Launch the game executable in suspended mode
- Inject `p2p_network.dll` using CreateRemoteThread
- Resume game execution

## Detailed Usage

### Command Line Options

```powershell
p2p_injector.exe [options]

Options:
  --exe PATH         Path to game executable (default: client/2025-06-04_Speedrun_P2P.exe)
  --dll PATH         Path to p2p_network.dll (default: client/p2p_network.dll)
  --no-admin         Run without admin privileges (not recommended)
  --verbose          Enable verbose logging
  --help             Show this message
```

### Examples

**Custom executable:**
```powershell
p2p_injector.exe --exe "client/my_client.exe" --dll "client/p2p_network.dll"
```

**Verbose mode:**
```powershell
p2p_injector.exe --verbose
```

## Technical Details

### Injection Process

1. **Process Creation**: Launches target executable in suspended state
2. **Memory Allocation**: Allocates memory in target process for DLL path
3. **Path Writing**: Writes DLL path to allocated memory
4. **Thread Creation**: Creates remote thread calling LoadLibraryA
5. **Execution Resume**: Resumes main thread after successful injection

### Error Handling

The injector includes comprehensive error checking for:
- File existence verification
- Process creation failures
- Memory allocation errors
- Thread creation issues
- Privilege elevation requirements

### Admin Privileges

Admin privileges are required for:
- Process manipulation operations
- Memory allocation in foreign processes
- Thread creation in foreign processes

The injector automatically detects and requests UAC elevation when needed.

## Testing

### Test Scripts

**Comprehensive injection test:**
```powershell
.\test_dll_injection.ps1
```

**DLL loading verification:**
```powershell
.\test_dll_load.ps1
```

### Verification

Successful injection is confirmed by:
1. Creation of `p2p_dll.log` file
2. DLL initialization messages in the log
3. P2P network functionality in-game

## Troubleshooting

### Common Issues

**Error 740 (Requires elevation):**
- Run the injector as Administrator
- Or allow UAC prompt when requested

**DLL not found:**
- Verify `p2p_network.dll` exists in client directory
- Check DLL dependencies (spdlog.dll, WebView2Loader.dll)

**Process creation failed:**
- Verify target path exists
- Check file permissions

**Injection failed:**
- Anti-cheat systems may block injection
- Try different injection timing or methods

### Log Files

- `p2p_dll.log` - P2P network DLL logs
- Injector console output - Injection process details

## Alternative Methods

If DLL injection fails, consider:

1. **Manual Import Table Editing**: Use CFF Explorer or PE-bear
2. **Proxy DLL**: Replace system DLLs like winmm.dll
3. **AppInit_DLLs**: Registry-based loading (less reliable)

## Security Considerations

- Only use injectors from trusted sources
- Be aware of anti-cheat system interactions
- Admin privileges are required for legitimate reasons

## Source Code

The injector source code is available in `dll_injector.cpp` and includes:
- Admin privilege handling
- Comprehensive error checking
- Clean resource management
- Cross-platform compatibility

## Support

For issues with the injector:
1. Check the troubleshooting section
2. Verify file paths and permissions
3. Test with verbose mode for detailed logs
4. Consult the main P2P network documentation