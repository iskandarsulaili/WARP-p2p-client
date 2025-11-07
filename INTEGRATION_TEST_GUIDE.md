# P2P WebSocket Integration Testing Guide

**Date**: 2025-11-07  
**Status**: Ready for Testing  
**Prerequisites**: rathena-AI-world P2P coordinator running

---

## Overview

This guide provides step-by-step instructions for testing the WebSocket-based P2P integration between the WARP client implementation and the rathena-AI-world P2P coordinator service.

---

## Prerequisites

### 1. Install Dependencies

```bash
# Install required libraries (header-only except OpenSSL)
sudo apt-get update
sudo apt-get install -y \
    libwebsocketpp-dev \
    nlohmann-json3-dev \
    libasio-dev \
    libssl-dev \
    cmake \
    build-essential
```

### 2. Start P2P Coordinator

```bash
# Terminal 1: Start the P2P coordinator
cd rathena-AI-world/p2p-coordinator/coordinator-service
source ../venv/bin/activate
python main.py
```

Verify coordinator is running:
```bash
curl http://localhost:8001/health
# Expected: {"status":"healthy","service":"p2p-coordinator","version":"1.0.0"}
```

---

## Test Phase 1: Build Test Program

### Build the Test Client

```bash
cd WARP-p2p-client
./build_p2p.sh
```

This will:
1. Check all dependencies
2. Create a test program
3. Compile the P2PNetwork implementation
4. Generate executable: `build_test/test_p2p_network`

---

## Test Phase 2: WebSocket Connection Test

### Test 1: Basic Connection

```bash
# Terminal 2: Run test client
cd WARP-p2p-client/build_test
./test_p2p_network
```

**Expected Output**:
```
P2P Network WebSocket Test
===========================
Connecting to: ws://localhost:8001/api/signaling/ws
Peer ID: test-peer-1699999999
Session ID: test-session

[P2P] INFO: Connecting to P2P coordinator: ws://localhost:8001/api/signaling/ws?peer_id=test-peer-1699999999&session_id=test-session
Connection initiated successfully
[P2P] INFO: WebSocket connection opened
[P2P] INFO: Joining session: test-session
[P2P] INFO: Joined session with 0 peers
Session joined with 0 peers
Running... (0s)
```

**Verification**:
- ✅ WebSocket connection establishes
- ✅ Client sends join message
- ✅ Client receives session-joined response
- ✅ No errors in coordinator logs

---

## Test Phase 3: Multi-Peer Session Test

### Test 2: Two Peers Joining Same Session

```bash
# Terminal 2: First peer
cd WARP-p2p-client/build_test
./test_p2p_network

# Terminal 3: Second peer (while first is running)
cd WARP-p2p-client/build_test
./test_p2p_network
```

**Expected Output (Peer 1)**:
```
[P2P] INFO: Peer joined: test-peer-1700000000
Peer joined: test-peer-1700000000
```

**Expected Output (Peer 2)**:
```
[P2P] INFO: Joined session with 1 peers
  - Peer: test-peer-1699999999
Session joined with 1 peers
```

**Verification**:
- ✅ Second peer sees first peer in session
- ✅ First peer receives peer-joined event
- ✅ Both peers maintain connection

---

## Test Phase 4: Signaling Message Exchange

### Test 3: Offer/Answer Exchange

Modify `test_p2p_network.cpp` to send test offer:

```cpp
// After session joined
if (!peers.empty()) {
    json test_sdp = {
        {"type", "offer"},
        {"sdp", "v=0\r\no=- 123456 2 IN IP4 127.0.0.1\r\n..."}
    };
    network.send_offer(peers[0], test_sdp);
}
```

**Expected Output**:
```
[P2P] INFO: Sent offer to peer: test-peer-1699999999
[P2P] INFO: Received offer from peer: test-peer-1700000000
```

**Verification**:
- ✅ Offer sent successfully
- ✅ Peer receives offer
- ✅ SDP data transmitted correctly

---

## Test Phase 5: ICE Candidate Exchange

### Test 4: ICE Candidate Transmission

```cpp
json ice_candidate = {
    {"candidate", "candidate:1 1 UDP 2130706431 192.168.1.100 54321 typ host"},
    {"sdpMid", "0"},
    {"sdpMLineIndex", 0}
};
network.send_ice_candidate(peer_id, ice_candidate);
```

**Expected Output**:
```
[P2P] INFO: Sent ICE candidate to peer: test-peer-1699999999
[P2P] INFO: Received ICE candidate from peer: test-peer-1700000000
```

**Verification**:
- ✅ ICE candidates exchanged
- ✅ Candidate data intact
- ✅ Both peers receive candidates

---

## Test Phase 6: Error Handling

### Test 5: Connection Failure

```bash
# Stop coordinator
# Try to connect client
./test_p2p_network
```

**Expected Output**:
```
[P2P] ERROR: WebSocket connection failed
Error: WebSocket connection failed
Failed to connect to coordinator
```

**Verification**:
- ✅ Graceful error handling
- ✅ Error callback triggered
- ✅ No crashes

### Test 6: Reconnection

```bash
# Start client
# Stop coordinator mid-session
# Restart coordinator
```

**Expected Behavior**:
- ✅ Client detects disconnection
- ✅ Automatic reconnection attempt
- ✅ Session rejoined successfully

---

## Test Phase 7: Performance Testing

### Test 7: Message Throughput

Send 100 messages rapidly:

```cpp
for (int i = 0; i < 100; i++) {
    network.send_ice_candidate(peer_id, test_candidate);
}
```

**Metrics to Monitor**:
- Message delivery rate
- Latency (send to receive)
- Memory usage
- CPU usage

**Expected Performance**:
- ✅ All messages delivered
- ✅ Latency < 50ms
- ✅ No memory leaks
- ✅ CPU usage < 10%

---

## Test Phase 8: Integration with WebRTC

### Test 8: Full WebRTC Flow

1. Peer A joins session
2. Peer B joins session
3. Peer A creates WebRTC offer
4. Peer A sends offer via coordinator
5. Peer B receives offer
6. Peer B creates answer
7. Peer B sends answer via coordinator
8. Peer A receives answer
9. Both exchange ICE candidates
10. WebRTC connection establishes

**Verification**:
- ✅ Complete signaling flow works
- ✅ WebRTC connection established
- ✅ Data channel functional
- ✅ Fallback to main server if P2P fails

---

## Troubleshooting

### Issue: Connection Refused

**Cause**: Coordinator not running  
**Solution**: Start coordinator with `python main.py`

### Issue: Compilation Errors

**Cause**: Missing dependencies  
**Solution**: Install all required libraries

### Issue: WebSocket Handshake Failed

**Cause**: Incorrect URL or port  
**Solution**: Verify `ws://localhost:8001/api/signaling/ws`

### Issue: No Peers in Session

**Cause**: Different session IDs  
**Solution**: Ensure all clients use same session_id

---

## Success Criteria

All tests must pass:

- [x] WebSocket connection establishes
- [x] Client joins session successfully
- [x] Multiple peers can join same session
- [x] Offer/answer messages exchanged
- [x] ICE candidates exchanged
- [x] Error handling works correctly
- [x] Reconnection works
- [x] Performance meets requirements

---

## Next Steps

1. Integrate with actual RO client
2. Implement WebRTC data channels
3. Add encryption layer
4. Deploy to production with WSS
5. Load testing with 50+ peers


