# WARP P2P Admin Guide

## P2P Host Selection Features

### Majority-Based Host Selection

The new majority-based host selection system automatically determines the optimal host based on player group latencies. This feature ensures better gameplay experience for the majority of players.

#### Configuration

Edit `config/p2p_config.yml`:

```yaml
majority_host:
  enabled: true  # Enable/disable the feature globally
  thresholds:
    min_group_size: 3  # Minimum players to consider as a group
    latency_threshold_ms: 300  # Maximum acceptable latency
    improvement_threshold_ms: 50  # Required improvement to trigger switch
```

#### Admin Commands

- `/togglemajorityhost` - Toggle majority-based host selection on/off
- `/checklatency` - View current latency groups and statistics
- `/forcehost [playerid]` - Manually set a specific player as host
- `/resethost` - Return to automatic host selection

### rAthena Features Integration

The WARP client now includes built-in support for rAthena-specific features without requiring compilation.

#### Configuration

Edit `config/rathena_config.yml`:

```yaml
rathena:
  features:
    extended_classes: true  # Enable extended class support
    custom_stats: true  # Enable custom stats system
    dynamic_items: true  # Enable dynamic item system
    custom_npcs: true  # Enable custom NPC support
    custom_quests: true  # Enable custom quest system
```

#### Security Settings

```yaml
security:
  packet_validation: true  # Validate all custom packets
  encryption_layer: true  # Enable additional encryption
  integrity_check: true  # Enable packet integrity checks
```

#### Performance Optimization

```yaml
performance:
  packet_batching: true  # Enable packet batching
  compression_level: "adaptive"  # Automatic compression adjustment
  cache_custom_data: true  # Cache frequently used custom data
```

## Monitoring and Debugging

### Network Metrics

Monitor network performance using:
- `/netstat` - View current network statistics
- `/groupstats` - View latency groups information
- `/hostinfo` - View current host information

### Logging

Enable detailed logging in `config/logging.yml`:

```yaml
logging:
  majority_events: true  # Log host selection events
  host_switches: true  # Log host switch events
  group_metrics: true  # Log group latency metrics
  feature_usage: true  # Log rAthena feature usage
  packet_errors: true  # Log packet processing errors
```

### Troubleshooting

Common Issues:

1. Host switching too frequently:
   - Increase `stabilization_time_ms` in majority host settings
   - Increase `improvement_threshold_ms` value

2. High latency despite host selection:
   - Check `latency_threshold_ms` settings
   - Verify network conditions of host candidates
   - Consider adjusting group size thresholds

3. rAthena features not working:
   - Verify feature flags in rathena_config.yml
   - Check packet validation logs
   - Ensure compatibility with server version

## Best Practices

1. Host Selection:
   - Monitor latency patterns before adjusting thresholds
   - Use `/checklatency` regularly to verify grouping
   - Keep `stabilization_time_ms` above 10000ms to prevent frequent switches

2. Performance:
   - Enable packet batching for better network efficiency
   - Use adaptive compression for optimal performance
   - Monitor resource usage with `/sysinfo`

3. Security:
   - Keep packet validation enabled
   - Regularly check security logs
   - Update encryption keys periodically

## Support

For additional support:
- Check debug logs in `logs/debug.log`
- Use `/debugmode` for detailed diagnostics
- Monitor performance metrics in `logs/metrics.log`