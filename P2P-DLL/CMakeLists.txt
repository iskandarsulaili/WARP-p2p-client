cmake_minimum_required(VERSION 3.25)
project(P2PNetworkDLL VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# vcpkg integration handled by command line
# if(DEFINED ENV{VCPKG_ROOT})
#     set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
#         CACHE STRING "Vcpkg toolchain file")
# endif()

message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
message(STATUS "VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")


# Find packages
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
# Find Boost components needed by Boost.Beast (each is a separate vcpkg package)
find_package(Boost REQUIRED COMPONENTS system regex chrono random)
# Boost.Beast is header-only, included with Boost.Asio
find_package(httplib CONFIG REQUIRED)

# WebRTC library
find_package(LibDataChannel CONFIG REQUIRED)

# QUIC protocol (msquic)
find_package(msquic CONFIG REQUIRED)

# ED25519 cryptography (libsodium)
find_package(unofficial-sodium CONFIG REQUIRED)

# LZ4 compression
find_package(lz4 CONFIG REQUIRED)

# Brotli compression (required by cpp-httplib)
find_package(unofficial-brotli CONFIG REQUIRED)

# Detours for function hooking (manual configuration since vcpkg doesn't provide CMake files)
find_path(DETOURS_INCLUDE_DIRS "detours/detours.h")
find_library(DETOURS_LIBRARY detours REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    src/core/NetworkManager.cpp
    src/core/ConfigManager.cpp
    src/network/SignalingClient.cpp
    src/network/HttpClient.cpp
    src/network/PacketRouter.cpp
    src/network/NetworkHooks.cpp
    src/network/QuicTransport.cpp
    src/webrtc/WebRTCManager.cpp
    src/webrtc/WebRTCPeerConnection.cpp
    src/security/SecurityManager.cpp
    src/security/AuthManager.cpp
    src/bandwidth/BandwidthManager.cpp
    src/compression/CompressionManager.cpp
    src/utils/Logger.cpp
    src/DllMain.cpp
)

# Header files
set(HEADERS
    include/NetworkManager.h
    include/ConfigManager.h
    include/SignalingClient.h
    include/HttpClient.h
    include/AuthManager.h
    include/PacketRouter.h
    include/NetworkHooks.h
    include/QuicTransport.h
    include/ITransport.h
    include/WebRTCManager.h
    include/WebRTCPeerConnection.h
    include/SecurityManager.h
    include/BandwidthManager.h
    include/CompressionManager.h
    include/Logger.h
    include/Types.h
)

# Create DLL
add_library(p2p_network SHARED ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(p2p_network PRIVATE
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    OpenSSL::SSL
    OpenSSL::Crypto
    httplib::httplib
    Boost::system
    Boost::regex
    Boost::chrono
    Boost::random
    # Boost.Beast is header-only, no linking needed
    LibDataChannel::LibDataChannel
    msquic
    unofficial-sodium::sodium
    ${DETOURS_LIBRARY}
    ZLIB::ZLIB
    lz4::lz4
    ws2_32
    unofficial::brotli::brotlidec
    unofficial::brotli::brotlienc
    unofficial::brotli::brotlicommon
)

# Add include directories
target_include_directories(p2p_network PRIVATE ${DETOURS_INCLUDE_DIRS})

# Compiler options
if(MSVC)
    target_compile_options(p2p_network PRIVATE
        /W4          # Warning level 4
        /WX          # Treat warnings as errors
        /MP          # Multi-processor compilation
        /permissive- # Standards conformance
        /EHsc        # Enable standard C++ exception handling
        /utf-8       # Unicode source encoding
    )
    target_compile_definitions(p2p_network PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        target_compile_definitions(p2p_network PRIVATE _AMD64_)
    else()
        target_compile_definitions(p2p_network PRIVATE _X86_)
    endif()
else()
    target_compile_options(p2p_network PRIVATE
        -Wall
        -Wextra
        -Werror
        -pedantic
    )
endif()

# Build configuration specific options
target_compile_definitions(p2p_network PRIVATE
    $<$<CONFIG:Debug>:DEBUG_BUILD>
    $<$<CONFIG:Release>:NDEBUG>
)

# Install targets
install(TARGETS p2p_network
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${HEADERS}
    DESTINATION include/p2p_network
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/config/p2p_config.json
    DESTINATION bin
)

# Testing
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    # find_package(GTest CONFIG REQUIRED)
    add_subdirectory(tests)
endif()

# Documentation
option(BUILD_DOCS "Build documentation" OFF)
if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        add_custom_target(docs ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    endif()
endif()

# Package configuration
set(CPACK_PACKAGE_NAME "P2PNetworkDLL")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "rAthena AI World")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "P2P Network DLL for Ragnarok Online")
set(CPACK_GENERATOR "ZIP")
include(CPack)

