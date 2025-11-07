# RO Client P2P WebSocket Migration Guide

**Date**: 2025-11-07  
**Purpose**: Adapt WARP reference implementation to actual RO client architecture  
**Difficulty**: Intermediate to Advanced

---

## Overview

This guide provides detailed instructions for adapting the reference P2PNetwork implementation to work with the actual Ragnarok Online client codebase, including code transformations, architectural adaptations, and integration patterns.

---

## Architecture Mapping

### Reference Implementation → RO Client

| Reference Component | RO Client Equivalent | Location |
|---------------------|---------------------|----------|
| `P2PNetwork` | `CP2PNetworkManager` | `src/network/P2PNetworkManager.cpp` |
| `std::cout` logging | `LOG_INFO/LOG_ERROR` | `src/utils/Logger.cpp` |
| Generic callbacks | RO event system | `src/core/EventManager.cpp` |
| `json` namespace | `nlohmann::json` | Same (header-only) |
| WebSocket client | Same | Same (header-only) |

---

## Step 1: Namespace and Naming Conventions

### Reference Implementation
```cpp
class P2PNetwork {
    void connect(const std::string& url);
};
```

### RO Client Adaptation
```cpp
namespace RO {
namespace Network {

class CP2PNetworkManager {
public:
    bool Connect(const std::string& strURL);
    void Disconnect();
    
private:
    // Use RO client naming conventions
    bool m_bConnected;
    std::string m_strPeerID;
    std::string m_strSessionID;
};

} // namespace Network
} // namespace RO
```

**Migration Steps**:
1. Wrap in RO namespace
2. Use Hungarian notation (m_ prefix for members)
3. Use PascalCase for methods
4. Add C prefix to class names

---

## Step 2: Logging System Integration

### Reference Implementation
```cpp
std::cout << "[P2P] INFO: Connected to coordinator" << std::endl;
std::cerr << "[P2P] ERROR: Connection failed" << std::endl;
```

### RO Client Adaptation
```cpp
// Assuming RO client has a Logger class
#include "utils/Logger.hpp"

// Info logging
CLogger::GetInstance()->LogInfo("P2P", "Connected to coordinator");

// Error logging
CLogger::GetInstance()->LogError("P2P", "Connection failed: %s", error.c_str());

// Debug logging (only in debug builds)
#ifdef _DEBUG
    CLogger::GetInstance()->LogDebug("P2P", "WebSocket message received: %s", msg.c_str());
#endif
```

**Migration Steps**:
1. Replace all `std::cout` with `CLogger::LogInfo`
2. Replace all `std::cerr` with `CLogger::LogError`
3. Add debug logging for development
4. Use format strings instead of stream operators

---

## Step 3: Callback System Integration

### Reference Implementation
```cpp
using SessionJoinedCallback = std::function<void(const std::vector<std::string>&)>;

void on_session_joined(SessionJoinedCallback callback) {
    session_joined_callback_ = callback;
}

// Usage
network.on_session_joined([](const std::vector<std::string>& peers) {
    // Handle session joined
});
```

### RO Client Adaptation
```cpp
// Use RO client event system
#include "core/EventManager.hpp"

class CP2PNetworkManager {
public:
    // Register for events instead of callbacks
    void Initialize() {
        CEventManager::GetInstance()->RegisterHandler(
            EVENT_P2P_SESSION_JOINED,
            this,
            &CP2PNetworkManager::OnSessionJoined
        );
    }
    
private:
    void OnSessionJoined(CEvent* pEvent) {
        auto pData = static_cast<CP2PSessionJoinedEvent*>(pEvent);
        const std::vector<std::string>& peers = pData->GetPeers();
        
        // Handle session joined
        CLogger::GetInstance()->LogInfo("P2P", "Joined session with %d peers", peers.size());
    }
};

// Event class
class CP2PSessionJoinedEvent : public CEvent {
public:
    CP2PSessionJoinedEvent(const std::vector<std::string>& peers)
        : CEvent(EVENT_P2P_SESSION_JOINED), m_vecPeers(peers) {}
    
    const std::vector<std::string>& GetPeers() const { return m_vecPeers; }
    
private:
    std::vector<std::string> m_vecPeers;
};
```

**Migration Steps**:
1. Define event types in `EventTypes.hpp`
2. Create event classes for each callback type
3. Replace callbacks with event handlers
4. Register handlers in initialization

---

## Step 4: Thread Safety Integration

### Reference Implementation
```cpp
void P2PNetwork::poll() {
    ws_client_.poll();
}
```

### RO Client Adaptation
```cpp
// RO client likely has a main game loop
class CP2PNetworkManager {
public:
    void Update(float fDeltaTime) {
        // Called from main game loop
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_pWSClient) {
            m_pWSClient->poll();
        }
        
        ProcessPendingMessages();
    }
    
private:
    void ProcessPendingMessages() {
        // Process messages on main thread
        while (!m_queueMessages.empty()) {
            auto msg = m_queueMessages.front();
            m_queueMessages.pop();
            
            HandleMessage(msg);
        }
    }
    
    std::mutex m_mutex;
    std::queue<std::string> m_queueMessages;
};
```

**Migration Steps**:
1. Add Update() method called from game loop
2. Use mutex for thread safety
3. Queue messages from WebSocket thread
4. Process messages on main thread

---

## Step 5: Configuration System Integration

### Reference Implementation
```cpp
bool connect(const std::string& url, const std::string& peer_id);
```

### RO Client Adaptation
```cpp
// Assuming RO client has CConfig class
#include "config/Config.hpp"

class CP2PNetworkManager {
public:
    bool Initialize() {
        CConfig* pConfig = CConfig::GetInstance();
        
        // Load from config.ini
        m_strCoordinatorURL = pConfig->GetString("P2P", "CoordinatorURL", "ws://localhost:8001/api/signaling/ws");
        m_nConnectionTimeout = pConfig->GetInt("P2P", "ConnectionTimeout", 5000);
        m_nMaxRetries = pConfig->GetInt("P2P", "MaxRetries", 3);
        m_bEnableP2P = pConfig->GetBool("P2P", "EnableP2P", true);
        
        if (!m_bEnableP2P) {
            CLogger::GetInstance()->LogInfo("P2P", "P2P support disabled in configuration");
            return false;
        }
        
        return Connect();
    }
    
private:
    std::string m_strCoordinatorURL;
    int m_nConnectionTimeout;
    int m_nMaxRetries;
    bool m_bEnableP2P;
};
```

**Migration Steps**:
1. Load all settings from config.ini
2. Add validation for configuration values
3. Support runtime configuration changes
4. Add default values for all settings

---

## Step 6: Error Handling Integration

### Reference Implementation
```cpp
try {
    ws_client_.connect(url);
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

### RO Client Adaptation
```cpp
// RO client likely has custom exception types
#include "core/Exceptions.hpp"

class CP2PNetworkManager {
public:
    bool Connect() {
        try {
            m_pWSClient->connect(m_strCoordinatorURL);
            return true;
        }
        catch (const CNetworkException& e) {
            CLogger::GetInstance()->LogError("P2P", "Network error: %s", e.GetMessage());
            HandleConnectionError(e);
            return false;
        }
        catch (const CSecurityException& e) {
            CLogger::GetInstance()->LogError("P2P", "Security error: %s", e.GetMessage());
            // Security errors are critical - disable P2P
            m_bEnableP2P = false;
            return false;
        }
        catch (const std::exception& e) {
            CLogger::GetInstance()->LogError("P2P", "Unexpected error: %s", e.what());
            return false;
        }
    }
    
private:
    void HandleConnectionError(const CNetworkException& e) {
        if (m_nRetryCount < m_nMaxRetries) {
            m_nRetryCount++;
            CLogger::GetInstance()->LogInfo("P2P", "Retrying connection (%d/%d)", m_nRetryCount, m_nMaxRetries);
            
            // Schedule retry
            CTimerManager::GetInstance()->ScheduleTimer(
                m_nReconnectDelay,
                this,
                &CP2PNetworkManager::Connect
            );
        } else {
            CLogger::GetInstance()->LogError("P2P", "Max retries exceeded, disabling P2P");
            m_bEnableP2P = false;
        }
    }
    
    int m_nRetryCount;
};
```

**Migration Steps**:
1. Use RO client exception types
2. Implement retry logic with timers
3. Add graceful degradation
4. Log all errors appropriately

---

## Step 7: Memory Management Integration

### Reference Implementation
```cpp
std::unique_ptr<websocketpp::client<websocketpp::config::asio_tls_client>> ws_client_;
```

### RO Client Adaptation
```cpp
// RO client may use custom memory management
class CP2PNetworkManager {
public:
    CP2PNetworkManager()
        : m_pWSClient(nullptr)
        , m_bConnected(false)
    {
        // Allocate WebSocket client
        m_pWSClient = new websocketpp::client<websocketpp::config::asio_tls_client>();
    }
    
    ~CP2PNetworkManager() {
        Shutdown();
        
        // Clean up
        SAFE_DELETE(m_pWSClient);
    }
    
    void Shutdown() {
        if (m_pWSClient && m_bConnected) {
            Disconnect();
        }
    }
    
private:
    websocketpp::client<websocketpp::config::asio_tls_client>* m_pWSClient;
    bool m_bConnected;
};

// Macro for safe deletion (common in game engines)
#define SAFE_DELETE(p) { if(p) { delete (p); (p) = nullptr; } }
```

**Migration Steps**:
1. Use raw pointers if RO client doesn't support smart pointers
2. Implement proper cleanup in destructor
3. Use SAFE_DELETE macros
4. Ensure no memory leaks

---

## Step 8: Network Manager Integration

### Reference Implementation
```cpp
P2PNetwork network;
network.connect(url, peer_id, session_id);
```

### RO Client Adaptation
```cpp
// Integrate with existing CNetworkManager
class CNetworkManager {
public:
    bool Initialize() {
        // Initialize traditional networking
        if (!InitializeTraditionalNetwork()) {
            return false;
        }
        
        #ifdef ENABLE_P2P_SUPPORT
        // Initialize P2P networking
        m_pP2PManager = new CP2PNetworkManager();
        if (!m_pP2PManager->Initialize()) {
            CLogger::GetInstance()->LogWarn("Network", "P2P initialization failed, using traditional networking only");
            SAFE_DELETE(m_pP2PManager);
        }
        #endif
        
        return true;
    }
    
    bool ConnectToMap(int nMapID) {
        #ifdef ENABLE_P2P_SUPPORT
        // Try P2P first if available
        if (m_pP2PManager && m_pP2PManager->IsEnabled() && IsP2PZone(nMapID)) {
            if (m_pP2PManager->ConnectToSession(GetSessionIDForMap(nMapID))) {
                m_bUsingP2P = true;
                return true;
            }
            
            CLogger::GetInstance()->LogWarn("Network", "P2P connection failed, falling back to traditional");
        }
        #endif
        
        // Fallback to traditional networking
        m_bUsingP2P = false;
        return ConnectToTraditionalServer(nMapID);
    }
    
private:
    #ifdef ENABLE_P2P_SUPPORT
    CP2PNetworkManager* m_pP2PManager;
    bool m_bUsingP2P;
    #endif
};
```

**Migration Steps**:
1. Add P2P manager to existing NetworkManager
2. Implement fallback logic
3. Add P2P zone detection
4. Maintain backward compatibility

---

## Step 9: Build System Integration

### CMakeLists.txt Modifications

```cmake
# Add P2P support option
option(ENABLE_P2P "Enable P2P WebSocket support" ON)

if(ENABLE_P2P)
    message(STATUS "P2P WebSocket support enabled")
    add_definitions(-DENABLE_P2P_SUPPORT)
    
    # Add include directories
    include_directories(
        ${CMAKE_SOURCE_DIR}/3rdparty/websocketpp
        ${CMAKE_SOURCE_DIR}/3rdparty/json
        ${CMAKE_SOURCE_DIR}/3rdparty/asio
    )
    
    # Add P2P source files
    set(P2P_SOURCES
        src/network/P2PNetworkManager.cpp
        src/network/P2PSession.cpp
    )
    
    set(P2P_HEADERS
        src/network/P2PNetworkManager.hpp
        src/network/P2PSession.hpp
    )
    
    # Find OpenSSL
    find_package(OpenSSL REQUIRED)
    
    # Add to client sources
    list(APPEND CLIENT_SOURCES ${P2P_SOURCES})
    list(APPEND CLIENT_HEADERS ${P2P_HEADERS})
    
    # Link OpenSSL
    target_link_libraries(ROClient PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()
```

---

## Step 10: Testing Integration

### Unit Test Example

```cpp
#include "gtest/gtest.h"
#include "network/P2PNetworkManager.hpp"

class P2PNetworkTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_pManager = new CP2PNetworkManager();
    }
    
    void TearDown() override {
        SAFE_DELETE(m_pManager);
    }
    
    CP2PNetworkManager* m_pManager;
};

TEST_F(P2PNetworkTest, Initialization) {
    EXPECT_TRUE(m_pManager->Initialize());
}

TEST_F(P2PNetworkTest, Connection) {
    ASSERT_TRUE(m_pManager->Initialize());
    EXPECT_TRUE(m_pManager->Connect());
}

TEST_F(P2PNetworkTest, SessionJoin) {
    ASSERT_TRUE(m_pManager->Initialize());
    ASSERT_TRUE(m_pManager->Connect());
    EXPECT_TRUE(m_pManager->JoinSession("test-session"));
}
```

---

## Common Pitfalls and Solutions

### Pitfall 1: Thread Safety Issues
**Problem**: WebSocket callbacks on different thread than game loop  
**Solution**: Use message queue and process on main thread

### Pitfall 2: Memory Leaks
**Problem**: WebSocket client not properly cleaned up  
**Solution**: Implement proper RAII or manual cleanup in destructor

### Pitfall 3: Configuration Not Loading
**Problem**: P2P settings not read from config.ini  
**Solution**: Ensure config parser supports P2P section

### Pitfall 4: Linker Errors
**Problem**: OpenSSL symbols not found  
**Solution**: Verify OpenSSL is properly linked in CMakeLists.txt

### Pitfall 5: Compilation Errors
**Problem**: Header-only libraries not found  
**Solution**: Check include paths in CMakeLists.txt

---

## Validation Checklist

After migration, verify:

- [ ] Code compiles without errors
- [ ] No warnings related to P2P code
- [ ] WebSocket connection establishes
- [ ] Messages sent and received correctly
- [ ] Fallback to main server works
- [ ] No memory leaks detected
- [ ] Thread safety verified
- [ ] Configuration loading works
- [ ] Logging integrated properly
- [ ] Error handling works correctly

---

## Next Steps

1. Complete integration following this guide
2. Run comprehensive tests
3. Proceed to WebRTC Data Channel Implementation
4. Implement Security and Encryption Layer
5. Conduct Performance Optimization

