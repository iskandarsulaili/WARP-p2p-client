#!/bin/bash
# Build script for Ragnarok Online Client with P2P WebSocket Support

set -e  # Exit on error

echo "========================================="
echo "RO Client P2P WebSocket Build Script"
echo "========================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if running in WARP-p2p-client directory
if [ ! -f "IMPLEMENTATION_GUIDE.md" ]; then
    echo -e "${RED}Error: Must run from WARP-p2p-client directory${NC}"
    exit 1
fi

# Step 1: Check dependencies
echo -e "${YELLOW}Step 1: Checking dependencies...${NC}"

check_dependency() {
    if dpkg -l | grep -q "^ii  $1"; then
        echo -e "${GREEN}✓${NC} $1 installed"
        return 0
    else
        echo -e "${RED}✗${NC} $1 not installed"
        return 1
    fi
}

MISSING_DEPS=0

check_dependency "libwebsocketpp-dev" || MISSING_DEPS=1
check_dependency "nlohmann-json3-dev" || MISSING_DEPS=1
check_dependency "libasio-dev" || MISSING_DEPS=1
check_dependency "libssl-dev" || MISSING_DEPS=1
check_dependency "cmake" || MISSING_DEPS=1
check_dependency "g++" || MISSING_DEPS=1

if [ $MISSING_DEPS -eq 1 ]; then
    echo ""
    echo -e "${YELLOW}Installing missing dependencies...${NC}"
    sudo apt-get update
    sudo apt-get install -y \
        libwebsocketpp-dev \
        nlohmann-json3-dev \
        libasio-dev \
        libssl-dev \
        cmake \
        g++ \
        build-essential
fi

echo ""

# Step 2: Verify reference implementation files
echo -e "${YELLOW}Step 2: Verifying reference implementation files...${NC}"

if [ -f "Core/Network/P2PNetwork.hpp" ]; then
    echo -e "${GREEN}✓${NC} P2PNetwork.hpp found"
else
    echo -e "${RED}✗${NC} P2PNetwork.hpp not found"
    exit 1
fi

if [ -f "Core/Network/P2PNetwork.cpp" ]; then
    echo -e "${GREEN}✓${NC} P2PNetwork.cpp found"
else
    echo -e "${RED}✗${NC} P2PNetwork.cpp not found"
    exit 1
fi

echo ""

# Step 3: Create build directory
echo -e "${YELLOW}Step 3: Creating build directory...${NC}"
mkdir -p build_test
cd build_test

# Step 4: Create minimal test program
echo -e "${YELLOW}Step 4: Creating test program...${NC}"

cat > test_p2p_network.cpp << 'EOF'
#include "../Core/Network/P2PNetwork.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "P2P Network WebSocket Test" << std::endl;
    std::cout << "===========================" << std::endl;
    
    P2PNetwork network;
    
    // Set up event handlers
    network.on_session_joined([](const std::vector<std::string>& peers) {
        std::cout << "Session joined with " << peers.size() << " peers" << std::endl;
        for (const auto& peer : peers) {
            std::cout << "  - Peer: " << peer << std::endl;
        }
    });
    
    network.on_peer_joined([](const std::string& peer_id) {
        std::cout << "Peer joined: " << peer_id << std::endl;
    });
    
    network.on_error([](const std::string& error) {
        std::cerr << "Error: " << error << std::endl;
    });
    
    // Connect to coordinator
    std::string url = "ws://localhost:8001/api/signaling/ws";
    std::string peer_id = "test-peer-" + std::to_string(std::time(nullptr));
    std::string session_id = "test-session";
    
    std::cout << "\nConnecting to: " << url << std::endl;
    std::cout << "Peer ID: " << peer_id << std::endl;
    std::cout << "Session ID: " << session_id << std::endl;
    std::cout << "\nPress Ctrl+C to exit...\n" << std::endl;
    
    if (network.connect(url, peer_id, session_id)) {
        std::cout << "Connection initiated successfully" << std::endl;
        
        // Keep running for 30 seconds
        for (int i = 0; i < 30; i++) {
            network.poll();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            if (i % 5 == 0) {
                std::cout << "Running... (" << i << "s)" << std::endl;
            }
        }
        
        network.disconnect();
    } else {
        std::cerr << "Failed to connect to coordinator" << std::endl;
        return 1;
    }
    
    std::cout << "\nTest completed successfully" << std::endl;
    return 0;
}
EOF

# Step 5: Create CMakeLists.txt for test
cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.15)
project(P2PNetworkTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

add_executable(test_p2p_network 
    test_p2p_network.cpp
    ../Core/Network/P2PNetwork.cpp
)

target_include_directories(test_p2p_network PRIVATE
    /usr/include
    ..
)

target_link_libraries(test_p2p_network PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

target_compile_options(test_p2p_network PRIVATE -Wall -Wextra)
EOF

echo ""

# Step 6: Configure with CMake
echo -e "${YELLOW}Step 5: Configuring with CMake...${NC}"
cmake . || {
    echo -e "${RED}CMake configuration failed${NC}"
    exit 1
}

echo ""

# Step 7: Build
echo -e "${YELLOW}Step 6: Building test program...${NC}"
make || {
    echo -e "${RED}Build failed${NC}"
    exit 1
}

echo ""
echo -e "${GREEN}=========================================${NC}"
echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${GREEN}=========================================${NC}"
echo ""
echo "Test executable: build_test/test_p2p_network"
echo ""
echo "To run the test:"
echo "  1. Start the P2P coordinator:"
echo "     cd ../rathena-AI-world/p2p-coordinator/coordinator-service"
echo "     python main.py"
echo ""
echo "  2. Run the test program:"
echo "     ./build_test/test_p2p_network"
echo ""

