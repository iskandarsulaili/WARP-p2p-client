# Test CMakeLists.txt

# Find Google Test
find_package(GTest CONFIG REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

# Test sources
set(TEST_SOURCES
    test_config.cpp
)

# Create test executable
add_executable(p2p_tests ${TEST_SOURCES})

# Link libraries
target_link_libraries(p2p_tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Add ConfigManager source directly to tests (since we don't have a static lib yet)
target_sources(p2p_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core/ConfigManager.cpp
)

# Compiler options
if(MSVC)
    target_compile_options(p2p_tests PRIVATE /W4)
else()
    target_compile_options(p2p_tests PRIVATE -Wall -Wextra)
endif()

# Register tests
include(GoogleTest)
gtest_discover_tests(p2p_tests)

# Copy test config file to build directory
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/test_config.json
    ${CMAKE_CURRENT_BINARY_DIR}/test_config.json
    COPYONLY
)

