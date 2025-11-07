#include <windows.h>
#include "../include/ConfigManager.h"
#include "../include/Logger.h"
#include "../include/NetworkManager.h"
#include <string>
#include <filesystem>
#include <sstream>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

// Global state
namespace {
    bool g_initialized = false;
    bool g_p2p_active = false;
    std::string g_last_error;
    std::string g_status_json;
    HMODULE g_dll_module = nullptr;
}

/**
 * DLL Entry Point
 * 
 * Called when the DLL is loaded/unloaded by the process.
 */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            // DLL is being loaded
            DisableThreadLibraryCalls(hModule);
            g_dll_module = hModule;

            try {
                // Get DLL directory
                char dll_path[MAX_PATH];
                if (!GetModuleFileNameA(hModule, dll_path, MAX_PATH)) {
                    g_last_error = "Failed to get DLL path";
                    return FALSE;
                }

                fs::path dll_dir = fs::path(dll_path).parent_path();

                // Load configuration
                fs::path config_path = dll_dir / "p2p_config.json";
                auto& config_mgr = P2P::ConfigManager::GetInstance();

                if (!config_mgr.LoadFromFile(config_path.string())) {
                    g_last_error = "Failed to load configuration from: " + config_path.string();
                    return FALSE;
                }

                // Initialize logging
                auto& logger = P2P::Logger::GetInstance();
                P2P::LoggingConfig log_config = config_mgr.GetConfig().logging;

                // Set log file path relative to DLL directory
                if (log_config.file.empty() || log_config.file[0] != '/') {
                    log_config.file = (dll_dir / log_config.file).string();
                }

                if (!logger.Initialize(log_config)) {
                    g_last_error = "Failed to initialize logger";
                    return FALSE;
                }

                LOG_INFO("=== P2P Network DLL Loaded ===");
                LOG_INFO("DLL Path: " + std::string(dll_path));
                LOG_INFO("Config Path: " + config_path.string());

                // Initialize NetworkManager
                auto& net_mgr = P2P::NetworkManager::GetInstance();
                if (!net_mgr.Initialize()) {
                    g_last_error = "Failed to initialize NetworkManager";
                    LOG_ERROR(g_last_error);
                    return FALSE;
                }

                LOG_INFO("NetworkManager initialized successfully");

                // Check if P2P is enabled in config
                if (config_mgr.IsP2PEnabled()) {
                    LOG_INFO("P2P networking is ENABLED");
                    LOG_INFO("P2P will start when player logs in");
                    // Note: We don't start P2P here because we don't have player_id yet
                    // P2P will be started via P2P_Initialize() or when player logs in
                } else {
                    LOG_INFO("P2P networking is DISABLED in configuration");
                }

                g_initialized = true;
                LOG_INFO("=== P2P Network DLL Initialization Complete ===");

            } catch (const std::exception& e) {
                g_last_error = std::string("Exception during initialization: ") + e.what();
                return FALSE;
            } catch (...) {
                g_last_error = "Unknown exception during initialization";
                return FALSE;
            }

            break;
        }

        case DLL_THREAD_ATTACH:
            // New thread created - nothing to do
            break;

        case DLL_THREAD_DETACH:
            // Thread exiting - nothing to do
            break;

        case DLL_PROCESS_DETACH:
            // DLL is being unloaded
            if (g_initialized) {
                try {
                    LOG_INFO("=== P2P Network DLL Shutting Down ===");

                    // Stop P2P networking if active
                    if (g_p2p_active) {
                        auto& net_mgr = P2P::NetworkManager::GetInstance();
                        net_mgr.Stop();
                        g_p2p_active = false;
                        LOG_INFO("P2P networking stopped");
                    }

                    // Shutdown NetworkManager
                    auto& net_mgr = P2P::NetworkManager::GetInstance();
                    net_mgr.Shutdown();
                    LOG_INFO("NetworkManager shutdown complete");

                    // Shutdown logging (flush all logs)
                    LOG_INFO("=== P2P Network DLL Shutdown Complete ===");
                    auto& logger = P2P::Logger::GetInstance();
                    logger.Shutdown();

                    g_initialized = false;

                } catch (const std::exception& e) {
                    // Can't log here since logger might be shut down
                    g_last_error = std::string("Exception during shutdown: ") + e.what();
                } catch (...) {
                    g_last_error = "Unknown exception during shutdown";
                }
            }
            break;
    }

    return TRUE;
}

/**
 * Exported function for manual initialization (optional)
 *
 * Can be called by the RO client if needed for explicit initialization.
 *
 * @param config_path Path to configuration file (optional, can be NULL)
 * @return true if initialized successfully, false otherwise
 */
extern "C" __declspec(dllexport) bool P2P_Initialize(const char* config_path) {
    if (!g_initialized) {
        g_last_error = "DLL not initialized. DllMain must be called first.";
        return false;
    }

    try {
        // Reload configuration if path provided
        if (config_path != nullptr && config_path[0] != '\0') {
            auto& config_mgr = P2P::ConfigManager::GetInstance();
            if (!config_mgr.LoadFromFile(config_path)) {
                g_last_error = "Failed to reload configuration from: " + std::string(config_path);
                LOG_ERROR(g_last_error);
                return false;
            }
            LOG_INFO("Configuration reloaded from: " + std::string(config_path));
        }

        // Check if P2P is enabled
        auto& config_mgr = P2P::ConfigManager::GetInstance();
        if (!config_mgr.IsP2PEnabled()) {
            g_last_error = "P2P is disabled in configuration";
            LOG_WARN(g_last_error);
            return false;
        }

        // Start P2P networking
        // Note: This requires player_id and user_id which should be set via separate API
        // For now, we just mark as ready to start
        LOG_INFO("P2P_Initialize called - P2P ready to start");
        LOG_INFO("Call P2P_Start(player_id, user_id) to begin P2P networking");

        return true;

    } catch (const std::exception& e) {
        g_last_error = std::string("Exception in P2P_Initialize: ") + e.what();
        LOG_ERROR(g_last_error);
        return false;
    } catch (...) {
        g_last_error = "Unknown exception in P2P_Initialize";
        LOG_ERROR(g_last_error);
        return false;
    }
}

/**
 * Exported function to start P2P networking
 *
 * @param player_id Player identifier (character ID)
 * @param user_id User identifier (account ID)
 * @return true if started successfully, false otherwise
 */
extern "C" __declspec(dllexport) bool P2P_Start(const char* player_id, const char* user_id) {
    if (!g_initialized) {
        g_last_error = "DLL not initialized";
        return false;
    }

    if (g_p2p_active) {
        g_last_error = "P2P already active";
        LOG_WARN(g_last_error);
        return true; // Already started, not an error
    }

    try {
        auto& config_mgr = P2P::ConfigManager::GetInstance();
        if (!config_mgr.IsP2PEnabled()) {
            g_last_error = "P2P is disabled in configuration";
            LOG_WARN(g_last_error);
            return false;
        }

        if (player_id == nullptr || user_id == nullptr) {
            g_last_error = "Invalid player_id or user_id";
            LOG_ERROR(g_last_error);
            return false;
        }

        LOG_INFO("Starting P2P networking...");
        LOG_INFO("Player ID: " + std::string(player_id));
        LOG_INFO("User ID: " + std::string(user_id));

        auto& net_mgr = P2P::NetworkManager::GetInstance();
        if (!net_mgr.Start(player_id, user_id)) {
            g_last_error = "NetworkManager failed to start";
            LOG_ERROR(g_last_error);
            return false;
        }

        g_p2p_active = true;
        LOG_INFO("P2P networking started successfully");
        return true;

    } catch (const std::exception& e) {
        g_last_error = std::string("Exception in P2P_Start: ") + e.what();
        LOG_ERROR(g_last_error);
        return false;
    } catch (...) {
        g_last_error = "Unknown exception in P2P_Start";
        LOG_ERROR(g_last_error);
        return false;
    }
}

/**
 * Exported function for manual shutdown (optional)
 */
extern "C" __declspec(dllexport) void P2P_Shutdown() {
    if (!g_initialized) {
        return;
    }

    try {
        LOG_INFO("P2P_Shutdown called");

        if (g_p2p_active) {
            auto& net_mgr = P2P::NetworkManager::GetInstance();
            net_mgr.Stop();
            g_p2p_active = false;
            LOG_INFO("P2P networking stopped");
        }

    } catch (const std::exception& e) {
        g_last_error = std::string("Exception in P2P_Shutdown: ") + e.what();
        LOG_ERROR(g_last_error);
    } catch (...) {
        g_last_error = "Unknown exception in P2P_Shutdown";
        LOG_ERROR(g_last_error);
    }
}

/**
 * Exported function to check if P2P is enabled
 */
extern "C" __declspec(dllexport) bool P2P_IsEnabled() {
    if (!g_initialized) {
        return false;
    }

    try {
        auto& config_mgr = P2P::ConfigManager::GetInstance();
        return config_mgr.IsP2PEnabled();
    } catch (...) {
        return false;
    }
}

/**
 * Exported function to check if P2P is currently active
 */
extern "C" __declspec(dllexport) bool P2P_IsActive() {
    if (!g_initialized) {
        return false;
    }

    try {
        auto& net_mgr = P2P::NetworkManager::GetInstance();
        return net_mgr.IsActive();
    } catch (...) {
        return false;
    }
}

/**
 * Exported function to get P2P status as JSON string
 *
 * Returns a JSON string with current status information.
 * The returned pointer is valid until the next call to this function.
 *
 * @return JSON string with status information
 */
extern "C" __declspec(dllexport) const char* P2P_GetStatus() {
    try {
        json status;

        status["dll_initialized"] = g_initialized;
        status["p2p_active"] = g_p2p_active;
        status["last_error"] = g_last_error;

        if (g_initialized) {
            auto& config_mgr = P2P::ConfigManager::GetInstance();
            status["p2p_enabled"] = config_mgr.IsP2PEnabled();

            auto& net_mgr = P2P::NetworkManager::GetInstance();
            status["network_active"] = net_mgr.IsActive();

            // Add configuration info
            const auto& config = config_mgr.GetConfig();
            status["coordinator_url"] = config.coordinator.rest_api_url;
            status["max_peers"] = config.p2p.max_peers;
            status["encryption_enabled"] = config.security.enable_encryption;

        } else {
            status["p2p_enabled"] = false;
            status["network_active"] = false;
        }

        // Store in global variable to keep pointer valid
        g_status_json = status.dump();
        return g_status_json.c_str();

    } catch (const std::exception& e) {
        g_last_error = std::string("Exception in P2P_GetStatus: ") + e.what();
        g_status_json = "{\"error\":\"" + g_last_error + "\"}";
        return g_status_json.c_str();
    } catch (...) {
        g_status_json = "{\"error\":\"Unknown exception in P2P_GetStatus\"}";
        return g_status_json.c_str();
    }
}

/**
 * Exported function to get last error message
 *
 * @return Last error message string
 */
extern "C" __declspec(dllexport) const char* P2P_GetLastError() {
    return g_last_error.c_str();
}

