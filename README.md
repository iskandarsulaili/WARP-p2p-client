# Win App Revamp Package - P2P Client

<p align="center">
    <img src="Images/logo.png?raw=true" alt="Warp logo" width=128 height=128>
</p>

![License](https://img.shields.io/github/license/Neo-Mind/WARP)
![RepoSize](https://img.shields.io/github/repo-size/Neo-Mind/WARP)
![Commit](https://img.shields.io/github/last-commit/Neo-Mind/WARP)

WARP P2P Client extension for enabling P2P hosting features in the Ragnarok Online client.

## P2P Features

### Host Node Integration
* Automatic host discovery
* Connection management
* Host selection optimization
* Performance monitoring
* Network metrics collection

### Security Features
* Host verification
* Secure communication
* Performance validation
* Network integrity checks
* Host reliability scoring

### Client Optimizations
* Connection pooling
* Resource caching
* Network latency reduction
* Bandwidth optimization
* Memory management

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
```

## Documentation
* [P2P Client Patches](doc/p2p_client_patches.md)
* [Security Implementation](doc/security.md)
* [Network Optimization](doc/network.md)

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
    └── p2p_client_patches.md
```

## Requirements
* Windows client
* DirectX 9.0c+
* .NET Framework 4.5+

## Contributing
See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License
GNU General Public License v3.0
