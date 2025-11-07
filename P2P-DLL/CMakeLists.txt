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

# vcpkg integration
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# Find packages
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(websocketpp CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)

# Note: libwebrtc requires manual integration (see docs/WEBRTC_INTEGRATION.md)
# find_package(webrtc REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Source files
set(SOURCES
    src/core/NetworkManager.cpp
    src/core/ConfigManager.cpp
    src/network/SignalingClient.cpp
    src/network/HttpClient.cpp
    src/network/PacketRouter.cpp
    src/webrtc/WebRTCManager.cpp
    src/webrtc/WebRTCPeerConnection.cpp
    src/security/SecurityManager.cpp
    src/security/AuthManager.cpp
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
    include/WebRTCManager.h
    include/WebRTCPeerConnection.h
    include/SecurityManager.h
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
    websocketpp::websocketpp
    httplib::httplib
    # webrtc::webrtc  # Add when libwebrtc is integrated
)

# Compiler options
if(MSVC)
    target_compile_options(p2p_network PRIVATE
        /W4          # Warning level 4
        /WX          # Treat warnings as errors
        /MP          # Multi-processor compilation
        /permissive- # Standards conformance
    )
    target_compile_definitions(p2p_network PRIVATE
        _WIN32_WINNT=0x0A00  # Windows 10
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
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
    find_package(GTest CONFIG REQUIRED)
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

