# WARP P2P Client Patches Specification

## Overview

This document details the patches required to modify the Ragexe client for P2P hosting capabilities using the WARP patching system.

## Required Patches

### 1. Network Protocol Extension

```cpp
// Packet structure extensions
struct P2P_HOST_INFO {
    uint32 host_id;
    uint32 performance_score;
    uint16 max_players;
    uint16 current_players;
    uint32 uptime;
    uint8 host_type;  // 0 = P2P, 1 = VPS
};

struct P2P_MAP_SERVER {
    uint32 map_id;
    P2P_HOST_INFO host;
    uint32 connection_token;
    uint8 encryption_key[32];
};
```

### 2. System Requirements Check

```cpp
// Add to client initialization
bool check_p2p_host_eligibility() {
    SystemInfo sys;
    NetworkInfo net;
    
    return (
        sys.cpu_cores >= 4 &&
        sys.available_ram >= 8589934592 && // 8GB
        net.upload_speed >= 10485760 &&     // 10Mbps
        net.latency < 100
    );
}
```

### 3. Host Registration Protocol

```yaml
# WARP patch definition
patch_id: p2p_host_registration
type: function_hook
target: CConnection::Connect
code: |
    void register_as_p2p_host() {
        if (!check_p2p_host_eligibility())
            return;
            
        P2P_HOST_INFO info;
        info.performance_score = calculate_system_score();
        info.max_players = determine_capacity();
        
        send_to_coordinator(PACKET_P2P_HOST_REGISTER, &info);
    }
```

### 4. Map Server Implementation

```yaml
patch_id: p2p_map_server
type: code_injection
target: CClientInfo
code: |
    class P2PMapServer {
    private:
        std::vector<SessionData> player_sessions;
        MapData current_map;
        
    public:
        void initialize_map(uint32 map_id);
        void handle_player_updates();
        void process_movement();
        void sync_state();
        
        // Anti-cheat validation
        bool validate_movement(Position* pos);
        bool validate_action(ActionData* act);
    };
```

### 5. Secure Communication Layer

```yaml
patch_id: p2p_encryption
type: packet_hook
target: CConnection::SendPacket
code: |
    struct SecureChannel {
        uint8 session_key[32];
        uint32 sequence_number;
        
        void encrypt_packet(void* data, size_t len) {
            uint8 iv[16];
            generate_iv(iv);
            aes_256_gcm_encrypt(session_key, iv, data, len);
        }
        
        bool verify_packet(void* data, size_t len) {
            return verify_hmac(data, len, session_key);
        }
    };
```

### 6. Performance Monitoring

```yaml
patch_id: p2p_monitoring
type: timer_hook
interval: 5000  # 5 seconds
code: |
    void update_host_metrics() {
        P2P_HOST_METRICS metrics;
        metrics.cpu_usage = get_cpu_usage();
        metrics.memory_usage = get_memory_usage();
        metrics.network_latency = measure_latency();
        metrics.player_count = count_active_players();
        
        send_to_coordinator(PACKET_P2P_METRICS, &metrics);
    }
```

### 7. Failover Handling

```yaml
patch_id: p2p_failover
type: event_hook
target: CConnection::OnDisconnect
code: |
    void handle_host_failure() {
        // Save current state
        GameState state;
        serialize_state(&state);
        
        // Request new host
        send_to_coordinator(PACKET_P2P_HOST_REQUEST, current_map_id);
        
        // Wait for response and reconnect
        wait_for_new_host_and_restore(state);
    }
```

## Patch Application Order

1. Network Protocol Extension
2. System Requirements Check
3. Host Registration
4. Map Server Implementation
5. Secure Communication
6. Performance Monitoring
7. Failover Handling

## Security Considerations

1. All P2P communication must be encrypted using AES-256-GCM
2. Host verification using digital signatures
3. Regular integrity checks of map state
4. Rate limiting for all network operations
5. Validation of all incoming packets
6. Anti-tampering measures for client memory

## Performance Optimization

1. Efficient delta compression for state updates
2. Bandwidth optimization for player updates
3. Local caching of frequently accessed data
4. Asynchronous processing for non-critical operations

## Integration Testing

1. Test host eligibility detection
2. Verify secure communication
3. Validate state synchronization
4. Test failover scenarios
5. Measure performance impact
6. Verify anti-cheat measures

## Dependencies

- WARP Patching Framework 2.0+
- OpenSSL 1.1.1+
- zlib 1.2.11+

## Build Instructions

1. Place patch files in WARP-p2p-client/Patches/
2. Update Patches.yml with new entries
3. Run WARP patch compiler
4. Test patched client
5. Package for distribution

## Version Compatibility

- Tested with rAthena compatible clients
- Supports Ragexe 2022-04-06 or newer
- Requires WARP framework version 2.0 or higher

## Installation Verification

After patching, verify:
1. P2P hosting capability detection
2. Secure communication establishment
3. Map server functionality
4. Performance monitoring
5. Failover handling
6. Anti-cheat systems

## Troubleshooting

Common issues and solutions:
1. Host eligibility detection failure
   - Verify system requirements
   - Check performance metrics calculation

2. Connection issues
   - Verify network configuration
   - Check encryption setup
   - Validate communication protocols

3. State synchronization problems
   - Check delta compression
   - Verify packet ordering
   - Validate state consistency checks