#pragma once

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <memory>
#include <mutex>
#include <atomic>

namespace P2P {

/**
 * Overlay display modes
 */
enum class OverlayMode {
    BASIC,      // Small corner overlay: "P2P: Connected/Disconnected"
    CONNECTION, // Show peer count, latency, packet loss
    DEBUG       // Full technical info including per-peer metrics
};

/**
 * OverlayRenderer - DirectX 9 overlay for P2P status display
 * 
 * Hooks into the game's DirectX EndScene to render status information.
 * Cycles through display modes with F9 key.
 */
class OverlayRenderer {
public:
    /**
     * Get singleton instance
     */
    static OverlayRenderer& GetInstance();

    /**
     * Initialize the overlay renderer
     * @param game_window HWND of the game window (optional, will auto-detect if nullptr)
     * @return true if initialization succeeded
     */
    bool Initialize(HWND game_window = nullptr);

    /**
     * Shutdown the overlay renderer and unhook DirectX
     */
    void Shutdown();

    /**
     * Cycle to next overlay mode (BASIC → CONNECTION → DEBUG → BASIC)
     */
    void CycleMode();

    /**
     * Get current overlay mode
     */
    OverlayMode GetCurrentMode() const;

    /**
     * Set overlay enabled/disabled
     */
    void SetEnabled(bool enabled);

    /**
     * Check if overlay is enabled
     */
    bool IsEnabled() const;

    /**
     * Render overlay (called from hooked EndScene)
     * @param device DirectX device pointer
     */
    void Render(IDirect3DDevice9* device);

    /**
     * Handle device reset (called before/after device reset)
     * @param device DirectX device pointer
     * @param before true if before reset, false if after reset
     */
    void OnDeviceReset(IDirect3DDevice9* device, bool before);

private:
    OverlayRenderer();
    ~OverlayRenderer();
    OverlayRenderer(const OverlayRenderer&) = delete;
    OverlayRenderer& operator=(const OverlayRenderer&) = delete;

    /**
     * Initialize DirectX font
     */
    bool InitializeFont(IDirect3DDevice9* device);

    /**
     * Release DirectX resources
     */
    void ReleaseResources();

    /**
     * Render basic mode overlay
     */
    void RenderBasicMode(IDirect3DDevice9* device);

    /**
     * Render connection mode overlay
     */
    void RenderConnectionMode(IDirect3DDevice9* device);

    /**
     * Render debug mode overlay
     */
    void RenderDebugMode(IDirect3DDevice9* device);

    /**
     * Draw text at position
     */
    void DrawText(const std::string& text, int x, int y, D3DCOLOR color);

    /**
     * Format bytes to human-readable string
     */
    std::string FormatBytes(uint64_t bytes) const;

    struct Impl;
    std::unique_ptr<Impl> impl_;

    mutable std::mutex mutex_;
    std::atomic<OverlayMode> current_mode_{OverlayMode::BASIC};
    std::atomic<bool> enabled_{true};
    std::atomic<bool> initialized_{false};
};

} // namespace P2P