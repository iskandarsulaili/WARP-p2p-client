# P2P Database Client Patches

This document describes how to apply and configure the P2P database patches for the client using WARP.

## Overview

The P2P database patches enable the client to work with our distributed database architecture, providing:
- Seamless failover support
- Distributed query handling
- Connection pool management
- Performance monitoring

## Installation

1. Copy `p2p_database.yml` to your WARP patches directory:
```bash
cp p2p_database.yml /path/to/WARP/Patches/
```

2. Enable the patch in `universal_p2p.yml`:
```yaml
enabled_patches:
  - p2p_database_support
```

3. Apply patches using WARP:
```bash
warp apply --patch p2p_database_support
```

## Configuration

### Network Settings
```yaml
network:
  enable_p2p: true
  failover_timeout: 5000  # milliseconds
  retry_attempts: 3
  connection_pools: 2
```

### Query Settings
```yaml
distributed_queries:
  enable_caching: true
  cache_timeout: 300000  # 5 minutes
  batch_size: 100
  max_retries: 3
```

### Monitoring
```yaml
monitoring:
  enable_metrics: true
  report_interval: 60000  # 1 minute
  log_level: info
```

## Required Files

The patch requires these DLLs to be present:
- network.dll
- dbclient.dll

Optional DLLs for enhanced functionality:
- metrics.dll
- profiler.dll

## Client Compatibility

- Minimum client version: 20200101
- Maximum client version: 20250101
- Tested with client builds:
  - 2022-04-06_Ragexe_1648707856
  - 2023-01-15_Ragexe_1673789012

## Features

### 1. Automatic Failover
The patch enables automatic failover when the primary database becomes unavailable:
- Detects connection failures
- Switches to replica servers
- Maintains session state
- Retries failed operations

### 2. Distributed Queries
Supports distributed query execution:
- Load balancing across nodes
- Query batching
- Result caching
- Transaction management

### 3. Connection Management
Implements connection pooling:
- Multiple connection pools
- Connection reuse
- Automatic reconnection
- Load distribution

## Troubleshooting

### Common Issues

1. Connection Failures
```
Error: Failed to connect to database server
Solution: Check network configuration and verify server addresses
```

2. Performance Issues
```
Error: Slow query responses
Solution: Adjust batch_size and connection_pools settings
```

3. Patch Application Failures
```
Error: Invalid patch checksum
Solution: Verify client version and DLL compatibility
```

### Logging

Logs are written to `p2p_database.log` with the following format:
```
[TIMESTAMP] [LEVEL] [COMPONENT] Message
```

Example:
```
[2025-03-08 05:38:30] [INFO] [ConnectionPool] Established connection to primary
```

### Monitoring

Monitor patch performance using:
```bash
warp status --component p2p_database
```

View metrics in real-time:
```bash
warp metrics --watch p2p_database
```

## Recovery Procedures

### Manual Failover
```bash
warp database --force-failover
```

### Cache Reset
```bash
warp database --clear-cache
```

### Connection Reset
```bash
warp database --reset-connections
```

## Security

The patch implements:
- Encrypted connections
- Query validation
- Token-based authentication
- Session persistence

## Performance Tuning

### Connection Pool Settings
```yaml
connection_pools:
  min_size: 2
  max_size: 10
  idle_timeout: 300
```

### Query Cache Settings
```yaml
query_cache:
  size: 1000
  ttl: 300
  cleanup_interval: 60
```

## Version History

- 1.0.0: Initial release
- 1.0.1: Added connection pooling
- 1.0.2: Enhanced failover handling
- 1.1.0: Added distributed query support

## Support

For issues and questions:
- Submit issues: https://github.com/rathena-AI-world/issues
- Documentation: https://docs.rathena-ai-world.com/p2p
- Community forum: https://forum.rathena-ai-world.com/p2p

## License

This patch is part of the rAthena AI World project and is licensed under the same terms as the main project.