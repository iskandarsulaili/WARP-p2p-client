#include "../../include/overlay/KeyboardHook.h"
#include "../../include/overlay/OverlayRenderer.h"
#include "../../include/Logger.h"

namespace P2P {

// Static instance pointer for callback access
static KeyboardHook* g_hook_instance = nullptr;

KeyboardHook::KeyboardHook() 
    : hook_handle_(nullptr)
    , dll_module_(nullptr) {
    g_hook_instance = this;
}

KeyboardHook::~KeyboardHook() {
    Uninstall();
    g_hook_instance = nullptr;
}

KeyboardHook& KeyboardHook::GetInstance() {
    static KeyboardHook instance;
    return instance;
}

bool KeyboardHook::Install(HMODULE dll_module) {
    if (installed_.load()) {
        LOG_WARN("KeyboardHook already installed");
        return true;
    }
    
    dll_module_ = dll_module;
    
    // Install low-level keyboard hook
    hook_handle_ = SetWindowsHookExW(
        WH_KEYBOARD_LL,
        LowLevelKeyboardProc,
        dll_module_,
        0  // All threads
    );
    
    if (!hook_handle_) {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to install keyboard hook. Error: " + std::to_string(error));
        return false;
    }
    
    installed_.store(true);
    LOG_INFO("Keyboard hook installed successfully (F9 for overlay mode cycling)");
    return true;
}

void KeyboardHook::Uninstall() {
    if (!installed_.load()) {
        return;
    }
    
    if (hook_handle_) {
        if (UnhookWindowsHookEx(hook_handle_)) {
            LOG_INFO("Keyboard hook uninstalled");
        } else {
            LOG_ERROR("Failed to uninstall keyboard hook");
        }
        hook_handle_ = nullptr;
    }
    
    installed_.store(false);
}

bool KeyboardHook::IsInstalled() const {
    return installed_.load();
}

LRESULT CALLBACK KeyboardHook::LowLevelKeyboardProc(
    int nCode, 
    WPARAM wParam, 
    LPARAM lParam) {
    
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kbd = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        
        // Check for F9 key press (key down event)
        if (wParam == WM_KEYDOWN && kbd->vkCode == VK_F9) {
            try {
                // Cycle overlay mode
                auto& overlay = OverlayRenderer::GetInstance();
                overlay.CycleMode();
                
                // Get current mode for logging
                OverlayMode mode = overlay.GetCurrentMode();
                const char* mode_name = "Unknown";
                switch (mode) {
                    case OverlayMode::BASIC:
                        mode_name = "BASIC";
                        break;
                    case OverlayMode::CONNECTION:
                        mode_name = "CONNECTION";
                        break;
                    case OverlayMode::DEBUG:
                        mode_name = "DEBUG";
                        break;
                }
                
                LOG_INFO(std::string("F9 pressed - Overlay mode: ") + mode_name);
            } catch (const std::exception& e) {
                LOG_ERROR(std::string("Error in keyboard hook: ") + e.what());
            }
        }
    }
    
    // Call next hook in chain
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

} // namespace P2P