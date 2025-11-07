#include <windows.h>
#include "ConfigManager.h"
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

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
            
            // Get DLL directory
            char dll_path[MAX_PATH];
            GetModuleFileNameA(hModule, dll_path, MAX_PATH);
            fs::path dll_dir = fs::path(dll_path).parent_path();
            
            // Load configuration
            fs::path config_path = dll_dir / "p2p_config.json";
            auto& config_mgr = P2P::ConfigManager::GetInstance();
            
            if (!config_mgr.LoadFromFile(config_path.string())) {
                // Failed to load config - use defaults
                // TODO: Log error
                return FALSE;
            }
            
            // TODO: Initialize logging
            // TODO: Initialize NetworkManager
            // TODO: Start P2P networking
            
            break;
        }
        
        case DLL_THREAD_ATTACH:
            // New thread created
            break;
            
        case DLL_THREAD_DETACH:
            // Thread exiting
            break;
            
        case DLL_PROCESS_DETACH:
            // DLL is being unloaded
            // TODO: Cleanup NetworkManager
            // TODO: Shutdown P2P networking
            // TODO: Cleanup logging
            break;
    }
    
    return TRUE;
}

/**
 * Exported function for manual initialization (optional)
 * 
 * Can be called by the RO client if needed for explicit initialization.
 */
extern "C" __declspec(dllexport) bool P2P_Initialize(const char* config_path) {
    auto& config_mgr = P2P::ConfigManager::GetInstance();
    
    if (!config_mgr.LoadFromFile(config_path)) {
        return false;
    }
    
    // TODO: Initialize components
    
    return true;
}

/**
 * Exported function for manual shutdown (optional)
 */
extern "C" __declspec(dllexport) void P2P_Shutdown() {
    // TODO: Cleanup components
}

/**
 * Exported function to check if P2P is enabled
 */
extern "C" __declspec(dllexport) bool P2P_IsEnabled() {
    auto& config_mgr = P2P::ConfigManager::GetInstance();
    return config_mgr.IsP2PEnabled();
}

/**
 * Exported function to get P2P status
 */
extern "C" __declspec(dllexport) const char* P2P_GetStatus() {
    // TODO: Return actual status
    return "Initialized";
}

