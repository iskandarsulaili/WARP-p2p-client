# P2P Database Client Patches

This document describes how to apply the DLL-free P2P database patches for the client using WARP.

## Overview

The P2P database patches modify the client executable directly to support distributed database functionality without requiring additional DLLs. This approach:
- Reduces dependencies
- Improves compatibility
- Simplifies deployment
- Maintains client integrity

## Installation

1. Back up your original client executable:
```bash
cp Client.exe Client.exe.backup
```

2. Apply the patch using WARP:
```bash
warp apply --patch p2p_database_support
```

## Binary Patch Details

### Network Handler (0x245A80)
Modifies the network initialization code to support P2P connections:
```nasm
; Original code
55                 ; push ebp
8B EC              ; mov ebp, esp
83 EC 18           ; sub esp, 18h

; Patched code
55                 ; push ebp
8B EC              ; mov ebp, esp
83 EC 20           ; sub esp, 20h
53                 ; push ebx
56                 ; push esi
57                 ; push edi
E8 ?? ?? ?? ??     ; call p2p_init
```

### Memory Layout

The patch uses a dedicated memory region for P2P functionality:
```
0x7A1000 - Server list (64 bytes)
0x7A1040 - Connection state (4 bytes)
0x7A1044 - Active server index (4 bytes)
0x7A1048 - Retry counter (4 bytes)
0x7A104C - Last ping timestamp (8 bytes)
```

## Configuration

### Network Settings
```yaml
network:
  max_servers: 8        # Maximum P2P nodes
  retry_delay: 1000     # Milliseconds between retries
  timeout: 5000         # Connection timeout
  batch_size: 100      # Query batch size
```

### Failover Configuration
```yaml
failover:
  enabled: true        # Enable automatic failover
  max_retries: 3       # Maximum retry attempts
  min_servers: 1       # Minimum active servers
```

## Features

### 1. Direct Database Access
- Built-in connection handling
- No external DLL dependencies
- Optimized memory usage
- Native query processing

### 2. Automatic Failover
- In-memory server tracking
- Quick failure detection
- Seamless server switching
- Connection state preservation

### 3. Performance Optimization
- Connection pooling
- Query batching
- Minimal memory footprint
- Efficient resource usage

## Verification

After patching, verify the installation:
```bash
warp verify --patch p2p_database_support
```

Expected output:
```
✓ Client executable checksum verified
✓ Memory regions allocated
✓ Network handler patched
✓ Query router installed
```

## Troubleshooting

### Common Issues

1. Patch Application Failure
```
Error: Invalid memory region
Solution: Ensure client version matches supported versions
```

2. Connection Issues
```
Error: Failed to initialize P2P
Solution: Verify memory permissions and antivirus settings
```

### Recovery

1. Restore original client:
```bash
cp Client.exe.backup Client.exe
```

2. Reset patch state:
```bash
warp reset --patch p2p_database_support
```

## Security

The patch implements security through:
- Code integrity checks
- Memory protection
- Input validation
- Session encryption

## Version Compatibility

### Supported Client Versions
- 2022-04-06_Ragexe_1648707856
- Future versions require checksum update

### Memory Requirements
- Minimum: 8MB free space
- Recommended: 16MB free space
- Virtual memory: 1MB reserved

## Performance Monitoring

Monitor patch performance using the built-in tools:
```bash
warp status --component p2p
warp metrics --watch connection
```

## Limitations

1. Memory Constraints
- Fixed server list size (8 entries)
- Limited connection pool (16 connections)
- Static memory allocation

2. Compatibility
- No support for encrypted clients
- Region-specific offsets required
- Some anticheat systems may interfere

## Best Practices

1. Installation
- Always backup client first
- Verify patch checksums
- Test in safe environment

2. Operation
- Monitor connection status
- Track performance metrics
- Keep configurations up to date

## Support

For technical assistance:
- GitHub Issues: [P2P Database Support](https://github.com/rathena-AI-world/issues)
- Documentation: [P2P System Guide](https://docs.rathena-ai-world.com/p2p)
- Forums: [Technical Support](https://forum.rathena-ai-world.com/p2p)