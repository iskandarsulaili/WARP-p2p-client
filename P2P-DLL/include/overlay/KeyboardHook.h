#pragma once

#include <windows.h>
#include <atomic>

namespace P2P {

/**
 * KeyboardHook - Low-level keyboard hook for F9 overlay mode cycling
 * 
 * Installs a Windows WH_KEYBOARD_LL hook to detect F9 key presses
 * and cycle through overlay display modes.
 */
class KeyboardHook {
public:
    /**
     * Get singleton instance
     */
    static KeyboardHook& GetInstance();

    /**
     * Install the keyboard hook
     * @param dll_module HMODULE of the DLL (for hook thread context)
     * @return true if installation succeeded
     */
    bool Install(HMODULE dll_module);

    /**
     * Uninstall the keyboard hook
     */
    void Uninstall();

    /**
     * Check if hook is installed
     */
    bool IsInstalled() const;

private:
    KeyboardHook();
    ~KeyboardHook();
    KeyboardHook(const KeyboardHook&) = delete;
    KeyboardHook& operator=(const KeyboardHook&) = delete;

    /**
     * Low-level keyboard hook callback
     */
    static LRESULT CALLBACK LowLevelKeyboardProc(
        int nCode, 
        WPARAM wParam, 
        LPARAM lParam
    );

    HHOOK hook_handle_;
    HMODULE dll_module_;
    std::atomic<bool> installed_{false};
};

} // namespace P2P