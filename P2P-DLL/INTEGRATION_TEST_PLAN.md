# P2P DLL Integration Test Plan

**Version**: 1.0
**Date**: 2025-11-07
**Status**: Draft

---

## Table of Contents

1. [Overview](#overview)
2. [Prerequisites](#prerequisites)
3. [Test Environment Setup](#test-environment-setup)
4. [Unit Tests](#unit-tests)
5. [Integration Tests](#integration-tests)
6. [Manual Test Procedures](#manual-test-procedures)
7. [Expected Results](#expected-results)
8. [Known Issues](#known-issues)
9. [Test Execution Checklist](#test-execution-checklist)

---

## 1. Overview

This document outlines the comprehensive testing strategy for the P2P Network DLL integration with the Ragnarok Online client via NEMO patcher.

### Testing Objectives

- Verify DLL loads correctly via NEMO patch
- Validate configuration loading and parsing
- Test component initialization and cleanup
- Verify graceful fallback to centralized server
- Test P2P connection establishment (when WebRTC is implemented)
- Validate security features (encryption, authentication)
- Test zone-based P2P activation
- Verify error handling and logging

### Testing Scope

**In Scope:**
- DLL loading and initialization
- Configuration management
- Logging functionality
- Component lifecycle management
- Error handling and recovery
- Graceful fallback mechanisms
- API endpoint integration (when coordinator is ready)

**Out of Scope (Known Limitations):**
- WebRTC peer connections (stub implementation)
- Actual P2P data transmission (requires WebRTC)
- JWT authentication (coordinator endpoint not implemented)
- Packet encryption (stub implementation)
- Performance benchmarking (requires real P2P traffic)

---

## 2. Prerequisites

### Software Requirements

1. **Build Tools**
   - CMake 3.25 or higher
   - Visual Studio 2022 (MSVC v143)
   - vcpkg package manager
   - Git

2. **Dependencies** (via vcpkg)
   - nlohmann-json 3.11.3
   - spdlog 1.12.0
   - websocketpp 0.8.2
   - cpp-httplib 0.14.3
   - openssl 3.0+
   - gtest 1.14.0 (for unit tests)

3. **NEMO Patcher**
   - NEMO v4.0+ (latest version)
   - Ragnarok Online client (2020+ recommended)

4. **P2P Coordinator Service** (Optional for full integration)
   - Python 3.11+
   - FastAPI 0.115.5
   - PostgreSQL 17
   - DragonflyDB

### Environment Setup

```bash
# Clone repository
git clone <repository-url>
cd WARP-p2p-client/P2P-DLL

# Install vcpkg dependencies
vcpkg install nlohmann-json spdlog websocketpp cpp-httplib openssl gtest

# Build DLL
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release

# Verify build output
ls bin/Release/p2p_network.dll
```

---

## 3. Test Environment Setup

### Test Configuration Files

Create test configuration files in `P2P-DLL/tests/config/`:

**test_p2p_config_enabled.json** - P2P enabled configuration
**test_p2p_config_disabled.json** - P2P disabled configuration
**test_p2p_config_invalid.json** - Invalid configuration for error testing

### Test Coordinator Setup (Optional)

```bash
# Start coordinator service
cd rathena-AI-world/p2p-coordinator
python -m venv venv
source venv/bin/activate  # Windows: venv\Scripts\activate
pip install -r requirements.txt
uvicorn coordinator-service.main:app --host 0.0.0.0 --port 8001
```

### Test Client Setup

1. Copy test RO client to `tests/client/`
2. Copy NEMO patcher to `tests/nemo/`
3. Create test directory structure:
   ```
   tests/
   ├── client/
   │   └── Warp.exe
   ├── nemo/
   │   └── NEMO.exe
   ├── config/
   │   ├── test_p2p_config_enabled.json
   │   ├── test_p2p_config_disabled.json
   │   └── test_p2p_config_invalid.json
   └── logs/
   ```

---

## 4. Unit Tests

### 4.1 ConfigManager Tests

**Test File**: `tests/ConfigManagerTest.cpp`

**Test Cases**:
- ✅ `LoadValidConfiguration` - Load valid JSON config
- ✅ `LoadInvalidConfiguration` - Handle malformed JSON
- ✅ `LoadMissingFile` - Handle missing config file
- ✅ `GetConfigValues` - Retrieve configuration values
- ✅ `IsP2PEnabled` - Check P2P enabled flag
- ✅ `DefaultValues` - Verify default configuration values

**Run Command**:
```bash
cd build
ctest -R ConfigManagerTest -V
```

### 4.2 Logger Tests

**Test File**: `tests/LoggerTest.cpp`

**Test Cases**:
- ✅ `InitializeLogger` - Initialize with valid config
- ✅ `LogLevels` - Test all log levels (TRACE, DEBUG, INFO, WARN, ERROR, FATAL)
- ✅ `FileOutput` - Verify log file creation and writing
- ✅ `ConsoleOutput` - Verify console logging
- ✅ `LogRotation` - Test log file rotation
- ✅ `AsyncLogging` - Test asynchronous logging mode

**Run Command**:
```bash
ctest -R LoggerTest -V
```

### 4.3 HttpClient Tests

**Test File**: `tests/HttpClientTest.cpp`

**Test Cases**:
- ✅ `GET_Request` - Perform GET request
- ✅ `POST_Request` - Perform POST request with JSON body
- ✅ `PUT_Request` - Perform PUT request
- ✅ `DELETE_Request` - Perform DELETE request
- ✅ `Authentication` - Test API key authentication
- ✅ `Timeout` - Test request timeout handling
- ✅ `ErrorHandling` - Test network error handling
- ✅ `InvalidURL` - Test invalid URL handling

**Run Command**:
```bash
ctest -R HttpClientTest -V
```

### 4.4 AuthManager Tests

**Test File**: `tests/AuthManagerTest.cpp`

**Test Cases**:
- ⚠️ `AcquireToken` - Acquire JWT token (WILL FAIL - endpoint not implemented)
- ⚠️ `RefreshToken` - Refresh expired token (WILL FAIL - endpoint not implemented)
- ⚠️ `ValidateToken` - Validate token (WILL FAIL - endpoint not implemented)
- ✅ `TokenStorage` - Test token storage and retrieval
- ✅ `ErrorHandling` - Test authentication error handling

**Expected Failures**: 3/5 tests will fail due to missing coordinator `/api/v1/auth/token` endpoint

**Run Command**:
```bash
ctest -R AuthManagerTest -V
```

### 4.5 SignalingClient Tests

**Test File**: `tests/SignalingClientTest.cpp`

**Test Cases**:
- ⚠️ `Connect` - Connect to WebSocket signaling server (WILL FAIL - no auth)
- ⚠️ `SendOffer` - Send WebRTC offer (WILL FAIL - no auth)
- ⚠️ `SendAnswer` - Send WebRTC answer (WILL FAIL - no auth)
- ⚠️ `SendICECandidate` - Send ICE candidate (WILL FAIL - no auth)
- ✅ `Reconnection` - Test reconnection logic
- ✅ `MessageSerialization` - Test JSON message serialization

**Expected Failures**: 4/6 tests will fail due to missing WebSocket authentication

**Run Command**:
```bash
ctest -R SignalingClientTest -V
```

### 4.6 WebRTCManager Tests

**Test File**: `tests/WebRTCManagerTest.cpp`

**Test Cases**:
- ⚠️ `CreatePeerConnection` - Create peer connection (STUB - returns fake data)
- ⚠️ `CreateOffer` - Create SDP offer (STUB - returns hardcoded SDP)
- ⚠️ `CreateAnswer` - Create SDP answer (STUB - returns hardcoded SDP)
- ⚠️ `AddICECandidate` - Add ICE candidate (STUB - does nothing)
- ⚠️ `SendData` - Send data via data channel (STUB - just logs)
- ✅ `PeerManagement` - Test peer tracking and management

**Expected Behavior**: All tests pass but functionality is MOCK/STUB

**Run Command**:
```bash
ctest -R WebRTCManagerTest -V
```

### 4.7 SecurityManager Tests

**Test File**: `tests/SecurityManagerTest.cpp`

**Test Cases**:
- ⚠️ `EncryptPacket` - Encrypt packet (STUB - no actual encryption)
- ⚠️ `DecryptPacket` - Decrypt packet (STUB - no actual decryption)
- ⚠️ `ValidatePacket` - Validate packet integrity (STUB - always returns true)
- ✅ `PacketSizeValidation` - Test packet size limits
- ✅ `RateLimiting` - Test rate limiting logic

**Expected Behavior**: Tests pass but encryption is NOT functional

**Run Command**:
```bash
ctest -R SecurityManagerTest -V
```

### 4.8 PacketRouter Tests

**Test File**: `tests/PacketRouterTest.cpp`

**Test Cases**:
- ✅ `RouteDecision_P2PEnabled` - Route decision when P2P enabled
- ✅ `RouteDecision_P2PDisabled` - Route decision when P2P disabled
- ✅ `RouteDecision_ZoneBased` - Route based on zone configuration
- ✅ `FallbackToServer` - Fallback when P2P fails
- ✅ `PacketInterception` - Intercept outgoing packets
- ✅ `TransparentFallback` - Transparent fallback to server

**Run Command**:
```bash
ctest -R PacketRouterTest -V
```

### 4.9 NetworkManager Tests

**Test File**: `tests/NetworkManagerTest.cpp`

**Test Cases**:
- ✅ `Initialize` - Initialize NetworkManager
- ✅ `Shutdown` - Shutdown NetworkManager
- ⚠️ `Start` - Start P2P networking (WILL FAIL - auth not implemented)
- ✅ `Stop` - Stop P2P networking
- ✅ `IsActive` - Check if P2P is active
- ✅ `OnZoneChange` - Handle zone change events

**Expected Failures**: 1/6 tests will fail due to authentication issues

**Run Command**:
```bash
ctest -R NetworkManagerTest -V
```

### 4.10 Run All Unit Tests

**Run Command**:
```bash
cd build
ctest --output-on-failure
```

**Expected Results**:
- **Total Tests**: ~50
- **Passing**: ~35 (70%)
- **Failing**: ~15 (30%)
- **Reason for Failures**: Missing coordinator endpoints, stub implementations

---

## 5. Integration Tests

### 5.1 DLL Loading Test

**Objective**: Verify DLL loads correctly via NEMO patch

**Prerequisites**:
- Built `p2p_network.dll` in `build/bin/Release/`
- NEMO patcher with LoadP2PDLL patch registered
- Test RO client

**Procedure**:
1. Copy `p2p_network.dll` to RO client directory
2. Copy `p2p_config.json` to RO client directory
3. Open NEMO patcher
4. Load RO client executable
5. Enable "Load P2P Network DLL" patch (should appear under "Network" category)
6. Apply patch and save patched client
7. Run patched client
8. Check `logs/p2p_network.log` for initialization messages

**Expected Results**:
```
[INFO] === P2P Network DLL Loaded ===
[INFO] DLL Path: C:\RO\p2p_network.dll
[INFO] Config Path: C:\RO\p2p_config.json
[INFO] NetworkManager initialized successfully
[INFO] P2P networking is ENABLED/DISABLED
[INFO] === P2P Network DLL Initialization Complete ===
```

**Pass Criteria**:
- ✅ DLL loads without errors
- ✅ Configuration file is read successfully
- ✅ Log file is created with initialization messages
- ✅ Client starts normally (no crashes)

### 5.2 Configuration Loading Test

**Objective**: Verify configuration loading and validation

**Test Cases**:

**5.2.1 Valid Configuration**
- Use `test_p2p_config_enabled.json`
- Expected: DLL initializes, P2P enabled

**5.2.2 P2P Disabled Configuration**
- Use `test_p2p_config_disabled.json`
- Expected: DLL initializes, P2P disabled, client works normally

**5.2.3 Invalid Configuration**
- Use `test_p2p_config_invalid.json` (malformed JSON)
- Expected: DLL fails to load, error logged, client may not start

**5.2.4 Missing Configuration**
- Remove `p2p_config.json`
- Expected: DLL fails to load, error logged

**Pass Criteria**:
- ✅ Valid config loads successfully
- ✅ Disabled config allows normal client operation
- ✅ Invalid config is detected and logged
- ✅ Missing config is detected and logged

### 5.3 Logging Test

**Objective**: Verify logging functionality

**Procedure**:
1. Configure logging in `p2p_config.json`:
   ```json
   "logging": {
     "level": "DEBUG",
     "file": "logs/p2p_network.log",
     "max_file_size_mb": 10,
     "max_files": 5,
     "console_output": true,
     "async_logging": true
   }
   ```
2. Run patched client
3. Perform various actions (login, zone change, etc.)
4. Check log file

**Expected Results**:
- ✅ Log file created at specified path
- ✅ Log messages include timestamps, levels, and messages
- ✅ Log rotation works when file size exceeds limit
- ✅ Console output appears (if enabled)


### 5.4 Graceful Fallback Test

**Objective**: Verify client works normally when P2P is disabled or fails

**Test Cases**:

**5.4.1 P2P Disabled in Config**
- Set `"p2p": { "enabled": false }` in config
- Expected: Client works normally, all traffic goes to server

**5.4.2 Coordinator Unreachable**
- Set invalid coordinator URL in config
- Expected: Client falls back to server, logs error

**5.4.3 Authentication Failure**
- Use invalid API key
- Expected: Client falls back to server, logs auth error

**Pass Criteria**:
- ✅ Client starts and runs normally in all scenarios
- ✅ No crashes or hangs
- ✅ All game functionality works (login, movement, combat, etc.)
- ✅ Errors are logged appropriately

### 5.5 Zone-Based P2P Test

**Objective**: Verify P2P activates only in configured zones

**Prerequisites**:
- P2P enabled in config
- Zone list configured: `"p2p_enabled_zones": ["prontera", "izlude"]`

**Procedure**:
1. Login to character
2. Start in non-P2P zone (e.g., "payon")
3. Check logs - P2P should NOT activate
4. Warp to P2P-enabled zone (e.g., "prontera")
5. Check logs - P2P should attempt to activate
6. Warp back to non-P2P zone
7. Check logs - P2P should deactivate

**Expected Results**:
```
[INFO] Zone changed: payon (P2P disabled for this zone)
[INFO] Zone changed: prontera (P2P enabled for this zone)
[INFO] Attempting to start P2P networking...
[INFO] Zone changed: payon (P2P disabled for this zone)
[INFO] Stopping P2P networking...
```

**Pass Criteria**:
- ✅ P2P only activates in configured zones
- ✅ Zone transitions work smoothly
- ✅ No crashes during zone changes

### 5.6 API Endpoint Integration Test

**Objective**: Test integration with coordinator REST API

**Prerequisites**:
- Coordinator service running on `http://localhost:8001`

**Test Cases**:

**5.6.1 Health Check**
- Endpoint: `GET /health`
- Expected: 200 OK, `{"status": "healthy"}`

**5.6.2 Session Creation** (⚠️ WILL FAIL - no authentication)
- Endpoint: `POST /api/v1/sessions`
- Expected: 401 Unauthorized (no JWT)

**5.6.3 Token Acquisition** (⚠️ WILL FAIL - endpoint not implemented)
- Endpoint: `POST /api/v1/auth/token`
- Expected: 404 Not Found

**Pass Criteria**:
- ✅ Health check succeeds
- ⚠️ Session creation fails with 401 (expected)
- ⚠️ Token endpoint returns 404 (expected - not implemented yet)

### 5.7 WebSocket Signaling Test

**Objective**: Test WebSocket connection to signaling server

**Prerequisites**:
- Coordinator service running
- WebSocket endpoint: `ws://localhost:8001/api/v1/signaling/ws`

**Procedure**:
1. Start P2P networking
2. Attempt WebSocket connection
3. Check logs for connection status

**Expected Results** (⚠️ WILL FAIL - no authentication):
```
[ERROR] WebSocket connection failed: 401 Unauthorized
[INFO] Falling back to centralized server
```

**Pass Criteria**:
- ⚠️ Connection fails with 401 (expected - no auth implemented)
- ✅ Graceful fallback occurs
- ✅ No crashes

---

## 6. Manual Test Procedures

### 6.1 NEMO Patch Application

**Objective**: Manually verify NEMO patch works correctly

**Steps**:
1. Open NEMO patcher
2. Click "Load Client"
3. Select RO client executable (e.g., `Warp.exe`)
4. Verify "Load P2P Network DLL" appears in patch list
5. Verify it's under "Network" category
6. Enable the patch (checkbox)
7. Click "Apply Patches"
8. Save patched client as `Warp_patched.exe`
9. Close NEMO

**Verification**:
- ✅ Patch appears in list
- ✅ Patch is under "Network" category
- ✅ Patch can be enabled/disabled
- ✅ Patched client is created successfully

### 6.2 DLL Deployment

**Objective**: Manually deploy DLL and configuration

**Steps**:
1. Copy `build/bin/Release/p2p_network.dll` to RO client directory
2. Copy `config/p2p_config.json` to RO client directory
3. Create `logs/` directory in RO client directory
4. Edit `p2p_config.json`:
   - Set coordinator URL
   - Set API key
   - Configure logging
   - Enable/disable P2P
5. Run patched client

**Verification**:
- ✅ DLL file exists in client directory
- ✅ Config file exists and is valid JSON
- ✅ Logs directory exists
- ✅ Client starts without errors

### 6.3 Client Startup Test

**Objective**: Verify client starts and runs normally

**Steps**:
1. Run patched client
2. Wait for client to fully load
3. Check `logs/p2p_network.log`
4. Login to game
5. Perform basic actions (walk, talk to NPC, etc.)
6. Check logs for any errors
7. Close client gracefully
8. Check logs for shutdown messages

**Verification**:
- ✅ Client starts without crashes
- ✅ Login works normally
- ✅ Game functionality works
- ✅ Logs show initialization and shutdown
- ✅ No error messages (or only expected errors)

### 6.4 P2P Status Check

**Objective**: Check P2P status via exported functions

**Steps**:
1. Create test program that loads DLL:
   ```cpp
   HMODULE dll = LoadLibrary("p2p_network.dll");
   auto P2P_GetStatus = (const char*(*)())GetProcAddress(dll, "P2P_GetStatus");
   const char* status = P2P_GetStatus();
   printf("Status: %s\n", status);
   ```
2. Run test program
3. Parse JSON status

**Expected Output**:
```json
{
  "dll_initialized": true,
  "p2p_active": false,
  "p2p_enabled": true,
  "network_active": false,
  "last_error": "",
  "coordinator_url": "http://localhost:8001",
  "max_peers": 50,
  "encryption_enabled": true
}
```

**Verification**:
- ✅ Status JSON is valid
- ✅ Fields match configuration
- ✅ `dll_initialized` is true

---

## 7. Expected Results

### 7.1 Success Criteria

**Minimum Viable Product (MVP)**:
- ✅ DLL loads via NEMO patch
- ✅ Configuration loads successfully
- ✅ Logging works
- ✅ Client runs normally with P2P disabled
- ✅ Graceful fallback works
- ✅ No crashes or hangs

**Full Functionality** (requires coordinator fixes):
- ⚠️ JWT authentication works
- ⚠️ WebSocket signaling connects
- ⚠️ WebRTC peer connections establish
- ⚠️ P2P data transmission works
- ⚠️ Packet encryption works

### 7.2 Known Limitations

**Current State** (as of 2025-11-07):
- ❌ WebRTC is STUB implementation (no real P2P connections)
- ❌ Encryption is STUB (no actual encryption)
- ❌ JWT authentication endpoint missing in coordinator
- ❌ WebSocket signaling has no authentication
- ❌ Packet validation is stub (always returns true)

**Impact**:
- Client will work normally but P2P will NOT function
- All traffic will fall back to centralized server
- This is EXPECTED and ACCEPTABLE for current phase

### 7.3 Test Results Summary

**Expected Test Results**:

| Test Category | Total | Pass | Fail | Skip |
|--------------|-------|------|------|------|
| Unit Tests | ~50 | ~35 | ~15 | 0 |
| Integration Tests | 7 | 4 | 3 | 0 |
| Manual Tests | 4 | 4 | 0 | 0 |
| **TOTAL** | **61** | **43** | **18** | **0** |

**Pass Rate**: 70% (acceptable given known limitations)

---

## 8. Known Issues

### 8.1 Critical Issues (from Code Review)

Reference: `COMPREHENSIVE_CODE_REVIEW.md`

**Authentication Issues**:
1. ❌ No JWT authentication endpoint in coordinator
2. ❌ WebSocket signaling has no authentication
3. ❌ Session endpoints have no authorization

**WebRTC Issues**:
4. ❌ WebRTC implementation is completely stub/mock
5. ❌ CreateOffer returns hardcoded fake SDP
6. ❌ SendData just logs, doesn't actually send

**Security Issues**:
7. ❌ EncryptPacket doesn't actually encrypt
8. ❌ DecryptPacket doesn't actually decrypt
9. ❌ ValidatePacket always returns true
10. ❌ Insecure default API keys in config

**Scalability Issues**:
11. ❌ Signaling state is in-memory (not scalable)
12. ❌ Cannot run multiple coordinator instances

**Configuration Issues**:
13. ❌ CORS mismatch (configured for 8000, runs on 8001)
14. ❌ No configuration validation

### 8.2 Expected Test Failures

**Tests That WILL Fail**:
- AuthManager: Token acquisition, refresh, validation (3 tests)
- SignalingClient: WebSocket connection, offer/answer/ICE (4 tests)
- NetworkManager: P2P start (1 test)
- Integration: Session creation, token endpoint, WebSocket (3 tests)

**Total Expected Failures**: 11 tests

**Reason**: Missing coordinator endpoints and stub implementations

### 8.3 Workarounds

**For Testing Without Coordinator**:
1. Set `"p2p": { "enabled": false }` in config
2. Client will work normally in server-only mode
3. All tests related to graceful fallback will pass

**For Testing With Coordinator**:
1. Start coordinator service
2. Expect authentication failures (logged, not fatal)
3. Client will fall back to server mode
4. Verify fallback works correctly

---

## 9. Test Execution Checklist

### Pre-Test Checklist

- [ ] Build environment set up (CMake, MSVC, vcpkg)
- [ ] All dependencies installed
- [ ] P2P DLL built successfully
- [ ] Unit tests compiled
- [ ] NEMO patcher available
- [ ] Test RO client available
- [ ] Test configuration files created
- [ ] Coordinator service running (optional)

### Unit Test Execution

- [ ] Run ConfigManager tests
- [ ] Run Logger tests
- [ ] Run HttpClient tests
- [ ] Run AuthManager tests (expect failures)
- [ ] Run SignalingClient tests (expect failures)
- [ ] Run WebRTCManager tests (stub behavior)
- [ ] Run SecurityManager tests (stub behavior)
- [ ] Run PacketRouter tests
- [ ] Run NetworkManager tests
- [ ] Review test results and document failures

### Integration Test Execution

- [ ] Test DLL loading via NEMO
- [ ] Test configuration loading (valid, invalid, missing)
- [ ] Test logging functionality
- [ ] Test graceful fallback (P2P disabled, coordinator down, auth failure)
- [ ] Test zone-based P2P activation
- [ ] Test API endpoint integration (expect failures)
- [ ] Test WebSocket signaling (expect failures)

### Manual Test Execution

- [ ] Apply NEMO patch manually
- [ ] Deploy DLL and configuration
- [ ] Test client startup
- [ ] Check P2P status via exported functions
- [ ] Verify client works normally
- [ ] Check logs for errors

### Post-Test Checklist

- [ ] Document all test results
- [ ] Categorize failures (expected vs unexpected)
- [ ] Create bug reports for unexpected failures
- [ ] Update known issues list
- [ ] Archive test logs
- [ ] Update test plan based on findings

---

## 10. Next Steps

### Immediate (Before Production)

1. **Fix Critical Issues** (Task 4)
   - Implement JWT authentication endpoint
   - Add WebSocket authentication
   - Implement real WebRTC (or clearly mark as Phase 2)
   - Implement real encryption (or clearly mark as Phase 2)

2. **Re-run Tests**
   - After fixes, re-run all failing tests
   - Target: 95%+ pass rate

3. **Performance Testing**
   - Load testing with multiple clients
   - Memory leak detection
   - CPU/network usage profiling

### Short-Term (1-2 weeks)

4. **Complete WebRTC Integration**
   - Replace stub with real libwebrtc
   - Test actual P2P connections
   - Measure latency and throughput

5. **Security Hardening**
   - Implement AES-256-GCM encryption
   - Add packet validation
   - Remove default API keys

### Medium-Term (2-4 weeks)

6. **Scalability Improvements**
   - Move signaling state to DragonflyDB
   - Enable horizontal scaling
   - Add load balancing

7. **Production Deployment**
   - Deploy to staging environment
   - Beta testing with real players
   - Monitor and fix issues

---

## Appendix A: Test Configuration Files

### test_p2p_config_enabled.json

```json
{
  "coordinator": {
    "rest_api_url": "http://localhost:8001",
    "websocket_url": "ws://localhost:8001/api/v1/signaling/ws",
    "timeout_seconds": 30,
    "reconnect_max_attempts": 5,
    "reconnect_backoff_ms": 1000
  },
  "p2p": {
    "enabled": true,
    "max_peers": 50,
    "max_packet_size_bytes": 65536,
    "max_bandwidth_mbps": 10,
    "target_bitrate_kbps": 1000,
    "enable_congestion_control": true,
    "packet_queue_size": 1000
  },
  "security": {
    "enable_encryption": true,
    "enable_authentication": true,
    "api_key": "test-api-key",
    "jwt_token": "",
    "certificate_validation": true,
    "tls_version": "1.3"
  },
  "logging": {
    "level": "DEBUG",
    "file": "logs/p2p_network.log",
    "max_file_size_mb": 10,
    "max_files": 5,
    "console_output": true,
    "async_logging": true
  },
  "zones": {
    "p2p_enabled_zones": ["prontera", "izlude", "geffen"],
    "fallback_on_failure": true,
    "zone_transition_timeout_ms": 5000
  }
}
```

### test_p2p_config_disabled.json

```json
{
  "p2p": {
    "enabled": false
  },
  "logging": {
    "level": "INFO",
    "file": "logs/p2p_network.log",
    "console_output": false
  }
}
```

---

**End of Integration Test Plan**

**Test Cases**:
- ✅ `LoadValidConfiguration` - Load valid JSON config
- ✅ `LoadInvalidConfiguration` - Handle malformed JSON
- ✅ `LoadMissingFile` - Handle missing config file
- ✅ `GetConfigValues` - Retrieve configuration values
- ✅ `IsP2PEnabled` - Check P2P enabled flag
- ✅ `DefaultValues` - Verify default configuration values

**Run Command**:
```bash
cd build
ctest -R ConfigManagerTest -V
```

### 4.2 Logger Tests

**Test File**: `tests/LoggerTest.cpp`

**Test Cases**:
- ✅ `InitializeLogger` - Initialize with valid config
- ✅ `LogLevels` - Test all log levels (TRACE, DEBUG, INFO, WARN, ERROR, FATAL)
- ✅ `FileOutput` - Verify log file creation and writing
- ✅ `ConsoleOutput` - Verify console logging
- ✅ `LogRotation` - Test log file rotation
- ✅ `AsyncLogging` - Test asynchronous logging mode

**Run Command**:
```bash
ctest -R LoggerTest -V
```


