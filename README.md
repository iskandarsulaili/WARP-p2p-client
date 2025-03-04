# Win App Revamp Package - P2P Client

<p align="center">
    <img src="Images/logo.png?raw=true" alt="Warp logo" width=128 height=128>
</p>

![License](https://img.shields.io/github/license/Neo-Mind/WARP)
![RepoSize](https://img.shields.io/github/repo-size/Neo-Mind/WARP)
![Commit](https://img.shields.io/github/last-commit/Neo-Mind/WARP)

WARP P2P Client extension for enabling P2P hosting features in the Ragnarok Online client. Designed to work seamlessly with [rAthena AI World](https://github.com/iskandarsulaili/rathena-AI-world) P2P hosting system and [FluxCP P2P Hosting](https://github.com/iskandarsulaili/FluxCP-AI-world-p2p-hosting) control panel.

## P2P Features

### Host Node Integration
* Automatic host discovery via rAthena coordinator
* Connection management with failover support
* AI-powered host selection optimization
* Real-time performance monitoring
* Network metrics collection for FluxCP dashboard
* Integration with rAthena's AI system for optimal host selection

### Security Features
* Host verification through rAthena coordinator
* Secure communication with P2P network
* Performance validation
* Network integrity checks
* Host reliability scoring system
* Integration with FluxCP security policies

### Client Optimizations
* Connection pooling
* Resource caching
* Network latency reduction
* Bandwidth optimization
* Memory management
* Adaptive performance tuning

## Installation

1. Copy patch files:
```
Patches/
└── p2p_hosting.yml    # P2P hosting configuration
```

2. Configure P2P settings in `p2p_hosting.yml`:
```yaml
p2p_client:
  enabled: true
  connection:
    timeout: 5000
    retry_attempts: 3
  monitoring:
    enabled: true
    update_interval: 60
  security:
    verify_hosts: true
    encryption: true
  rathena:
    coordinator_port: 5121
    metrics_enabled: true
  fluxcp:
    monitoring_enabled: true
    metrics_port: 8080
```

## Integration

### rAthena AI World
- Connects to rAthena's P2P coordinator service
- Supports AI-driven host selection
- Provides performance metrics for AI optimization
- Automatic failover to VPS when needed

### FluxCP Control Panel
- Real-time metrics reporting to FluxCP dashboard
- Security status monitoring
- Performance data visualization
- Host reliability reporting

## Documentation
* [P2P Client Patches](doc/p2p_client_patches.md)
* [Security Implementation](doc/security.md)
* [Network Optimization](doc/network.md)
* [rAthena Integration](doc/rathena_integration.md)
* [FluxCP Integration](doc/fluxcp_integration.md)

## Project Structure
```text
WARP/
├── README.md
├── LICENSE
├── Patches/
│   └── p2p_hosting.yml
├── Scripts/
│   ├── Support/
│   └── Patches/
└── doc/
    ├── p2p_client_patches.md
    ├── rathena_integration.md
    └── fluxcp_integration.md
```

## Requirements
* Windows client
* DirectX 9.0c+
* .NET Framework 4.5+
* rAthena AI World server (for P2P features)
* FluxCP Control Panel (for monitoring)

## Contributing
See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License
GNU General Public License v3.0

## Related Projects
- [rAthena AI World](https://github.com/iskandarsulaili/rathena-AI-world) - The main server implementation with P2P hosting and AI features
- [FluxCP P2P Hosting](https://github.com/iskandarsulaili/FluxCP-AI-world-p2p-hosting) - Control panel with P2P monitoring and management
