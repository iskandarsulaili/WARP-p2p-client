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

## Related Projects

### rAthena AI World

The **[rathena-AI-world](https://github.com/iskandarsulaili/rathena-AI-world)** is an enhanced rAthena MMORPG server with AI-driven autonomous NPCs and a P2P coordinator service. This WARP P2P client connects to the rathena-AI-world P2P coordinator to enable hybrid P2P architecture.

**Key Features**:
- AI-driven NPCs with personality-based behavior (Big Five model)
- Multi-agent AI system using CrewAI framework
- Dynamic quest generation and economic simulation
- P2P coordinator service for WebRTC signaling
- Long-term memory management with Memori SDK

**Integration**: This WARP client connects to the P2P coordinator service at `ws://localhost:8001/api/signaling/ws` (development) or `wss://coordinator.yourdomain.com/api/signaling/ws` (production). The coordinator handles WebRTC signaling, session management, and host selection.

**Architecture**: Hybrid P2P model where:
- **Centralized (rathena-AI-world)**: AI NPCs, authentication, anti-cheat, critical game logic
- **P2P (WARP client)**: Zone-based player interactions hosted by qualified players

**Integration Guide**: See the [P2P Integration Analysis](../P2P_INTEGRATION_ANALYSIS.md) for detailed compatibility assessment and required modifications.

---

## Documentation
* [P2P Client Patches](doc/p2p_client_patches.md) - Patch configuration and implementation
* [Implementation Guide](IMPLEMENTATION_GUIDE.md) - Complete implementation guide
* [Integration Test Guide](INTEGRATION_TEST_GUIDE.md) - Testing procedures
* [Production Deployment Guide](PRODUCTION_DEPLOYMENT_GUIDE.md) - Production deployment
* [Integration Analysis](../P2P_INTEGRATION_ANALYSIS.md) - Compatibility with rathena-AI-world coordinator

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
Contributions are welcome! Please ensure all code follows the existing style and includes appropriate tests.

## License
GNU General Public License v3.0
