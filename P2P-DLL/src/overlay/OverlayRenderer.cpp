#include "../../include/overlay/OverlayRenderer.h"
#include "../../include/NetworkManager.h"
#include "../../include/WebRTCManager.h"
#include "../../include/BandwidthManager.h"
#include "../../include/Logger.h"
#include "../../include/ConfigManager.h"
#include <detours/detours.h>
#include <sstream>
#include <iomanip>

namespace P2P {

// DirectX function pointer types
typedef HRESULT(WINAPI* EndScene_t)(IDirect3DDevice9*);
typedef HRESULT(WINAPI* Reset_t)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

// Original function pointers
static EndScene_t g_original_endscene = nullptr;
static Reset_t g_original_reset = nullptr;

// Global instance pointer for hook callbacks
static OverlayRenderer* g_overlay_instance = nullptr;

/**
 * Hooked EndScene function
 */
HRESULT WINAPI Hooked_EndScene(IDirect3DDevice9* device) {
    if (g_overlay_instance && g_overlay_instance->IsEnabled()) {
        g_overlay_instance->Render(device);
    }
    return g_original_endscene(device);
}

/**
 * Hooked Reset function (to handle device reset)
 */
HRESULT WINAPI Hooked_Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params) {
    if (g_overlay_instance) {
        g_overlay_instance->OnDeviceReset(device, true);
    }
    HRESULT result = g_original_reset(device, params);
    if (g_overlay_instance && SUCCEEDED(result)) {
        g_overlay_instance->OnDeviceReset(device, false);
    }
    return result;
}

/**
 * Implementation details
 */
struct OverlayRenderer::Impl {
    ID3DXFont* font = nullptr;
    IDirect3DDevice9* device = nullptr;
    HWND game_window = nullptr;
    bool hooks_installed = false;
    
    // Font parameters
    int font_height = 14;
    int font_weight = FW_NORMAL;
    std::string font_face = "Arial";
    
    // Display positions
    int overlay_x = 10;
    int overlay_y = 10;
    int line_height = 16;
    
    // Colors
    D3DCOLOR color_title = D3DCOLOR_ARGB(255, 255, 255, 0);    // Yellow
    D3DCOLOR color_connected = D3DCOLOR_ARGB(255, 0, 255, 0);   // Green
    D3DCOLOR color_disconnected = D3DCOLOR_ARGB(255, 255, 0, 0); // Red
    D3DCOLOR color_text = D3DCOLOR_ARGB(255, 255, 255, 255);    // White
    D3DCOLOR color_label = D3DCOLOR_ARGB(255, 200, 200, 200);   // Light gray
    
    ~Impl() {
        if (font) {
            font->Release();
            font = nullptr;
        }
    }
};

OverlayRenderer::OverlayRenderer() : impl_(std::make_unique<Impl>()) {
    g_overlay_instance = this;
}

OverlayRenderer::~OverlayRenderer() {
    Shutdown();
    g_overlay_instance = nullptr;
}

OverlayRenderer& OverlayRenderer::GetInstance() {
    static OverlayRenderer instance;
    return instance;
}

bool OverlayRenderer::Initialize(HWND game_window) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_.load()) {
        LOG_WARN("OverlayRenderer already initialized");
        return true;
    }
    
    impl_->game_window = game_window;
    
    // Note: DirectX device and font will be initialized on first Render() call
    // when we have access to the game's D3D device
    
    initialized_.store(true);
    LOG_INFO("OverlayRenderer initialized");
    return true;
}

void OverlayRenderer::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_.load()) {
        return;
    }
    
    ReleaseResources();
    
    // Unhook DirectX functions
    if (impl_->hooks_installed && g_original_endscene) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)g_original_endscene, Hooked_EndScene);
        if (g_original_reset) {
            DetourDetach(&(PVOID&)g_original_reset, Hooked_Reset);
        }
        DetourTransactionCommit();
        impl_->hooks_installed = false;
        LOG_INFO("DirectX hooks removed");
    }
    
    initialized_.store(false);
    LOG_INFO("OverlayRenderer shut down");
}

void OverlayRenderer::CycleMode() {
    OverlayMode current = current_mode_.load();
    OverlayMode next;
    
    switch (current) {
        case OverlayMode::BASIC:
            next = OverlayMode::CONNECTION;
            break;
        case OverlayMode::CONNECTION:
            next = OverlayMode::DEBUG;
            break;
        case OverlayMode::DEBUG:
        default:
            next = OverlayMode::BASIC;
            break;
    }
    
    current_mode_.store(next);
    LOG_INFO("Overlay mode changed to: " + 
             std::to_string(static_cast<int>(next)));
}

OverlayMode OverlayRenderer::GetCurrentMode() const {
    return current_mode_.load();
}

void OverlayRenderer::SetEnabled(bool enabled) {
    enabled_.store(enabled);
    LOG_INFO(std::string("Overlay ") + (enabled ? "enabled" : "disabled"));
}

bool OverlayRenderer::IsEnabled() const {
    return enabled_.load() && initialized_.load();
}

bool OverlayRenderer::InitializeFont(IDirect3DDevice9* device) {
    if (impl_->font) {
        return true; // Already initialized
    }
    
    HRESULT hr = D3DXCreateFontA(
        device,
        impl_->font_height,
        0,
        impl_->font_weight,
        1,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        impl_->font_face.c_str(),
        &impl_->font
    );
    
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create DirectX font: " + std::to_string(hr));
        return false;
    }
    
    impl_->device = device;
    
    // Install DirectX hooks if not already done
    if (!impl_->hooks_installed) {
        void** vtable = *reinterpret_cast<void***>(device);
        g_original_endscene = reinterpret_cast<EndScene_t>(vtable[42]); // EndScene is index 42
        g_original_reset = reinterpret_cast<Reset_t>(vtable[16]);       // Reset is index 16
        
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)g_original_endscene, Hooked_EndScene);
        DetourAttach(&(PVOID&)g_original_reset, Hooked_Reset);
        LONG error = DetourTransactionCommit();
        
        if (error == NO_ERROR) {
            impl_->hooks_installed = true;
            LOG_INFO("DirectX hooks installed successfully");
        } else {
            LOG_ERROR("Failed to install DirectX hooks: " + std::to_string(error));
            return false;
        }
    }
    
    return true;
}

void OverlayRenderer::ReleaseResources() {
    if (impl_->font) {
        impl_->font->Release();
        impl_->font = nullptr;
    }
    impl_->device = nullptr;
}

void OverlayRenderer::Render(IDirect3DDevice9* device) {
    if (!device || !enabled_.load()) {
        return;
    }
    
    // Initialize font on first render
    if (!impl_->font) {
        if (!InitializeFont(device)) {
            return;
        }
    }
    
    // Render based on current mode
    OverlayMode mode = current_mode_.load();
    try {
        switch (mode) {
            case OverlayMode::BASIC:
                RenderBasicMode(device);
                break;
            case OverlayMode::CONNECTION:
                RenderConnectionMode(device);
                break;
            case OverlayMode::DEBUG:
                RenderDebugMode(device);
                break;
        }
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Overlay render error: ") + e.what());
    }
}

void OverlayRenderer::OnDeviceReset(IDirect3DDevice9* device, bool before) {
    if (before) {
        // Release resources before reset
        if (impl_->font) {
            impl_->font->OnLostDevice();
        }
    } else {
        // Restore resources after reset
        if (impl_->font) {
            impl_->font->OnResetDevice();
        }
    }
}

void OverlayRenderer::RenderBasicMode(IDirect3DDevice9* /* device */) {
    auto& net_mgr = NetworkManager::GetInstance();
    bool is_active = net_mgr.IsActive();
    
    std::string status = "P2P: ";
    status += is_active ? "Connected" : "Disconnected";
    
    D3DCOLOR color = is_active ? impl_->color_connected : impl_->color_disconnected;
    DrawText(status, impl_->overlay_x, impl_->overlay_y, color);
}

void OverlayRenderer::RenderConnectionMode(IDirect3DDevice9* /* device */) {
    auto& net_mgr = NetworkManager::GetInstance();
    bool is_active = net_mgr.IsActive();
    
    int y = impl_->overlay_y;
    
    // Title
    DrawText("=== P2P Status ===", impl_->overlay_x, y, impl_->color_title);
    y += impl_->line_height;
    
    // Status
    std::string status = "Status: ";
    status += is_active ? "Connected" : "Disconnected";
    D3DCOLOR color = is_active ? impl_->color_connected : impl_->color_disconnected;
    DrawText(status, impl_->overlay_x, y, color);
    y += impl_->line_height;
    
    if (!is_active) {
        return;
    }
    
    // Get metrics
    auto& bw_mgr = net_mgr.GetBandwidthManager();
    auto metrics = bw_mgr.GetOverallMetrics();
    
    // Peer count (need to access WebRTCManager through NetworkManager implementation)
    DrawText("Peers: N/A", impl_->overlay_x, y, impl_->color_text);
    y += impl_->line_height;
    
    // Average latency
    std::ostringstream latency_ss;
    latency_ss << "Ping: " << std::fixed << std::setprecision(0) 
               << metrics.average_latency_ms << "ms";
    DrawText(latency_ss.str(), impl_->overlay_x, y, impl_->color_text);
    y += impl_->line_height;
    
    // Packet loss
    std::ostringstream loss_ss;
    loss_ss << "Loss: " << std::fixed << std::setprecision(2) 
            << metrics.packet_loss_percent << "%";
    DrawText(loss_ss.str(), impl_->overlay_x, y, impl_->color_text);
}

void OverlayRenderer::RenderDebugMode(IDirect3DDevice9* /* device */) {
    auto& net_mgr = NetworkManager::GetInstance();
    auto& config_mgr = ConfigManager::GetInstance();
    bool is_active = net_mgr.IsActive();
    
    int y = impl_->overlay_y;
    
    // Title
    DrawText("=== P2P Debug Info ===", impl_->overlay_x, y, impl_->color_title);
    y += impl_->line_height;
    
    // Status
    std::string status = "Status: ";
    status += is_active ? "Connected" : "Disconnected";
    D3DCOLOR color = is_active ? impl_->color_connected : impl_->color_disconnected;
    DrawText(status, impl_->overlay_x, y, color);
    y += impl_->line_height;
    
    if (!is_active) {
        return;
    }
    
    // Get metrics
    auto& bw_mgr = net_mgr.GetBandwidthManager();
    auto metrics = bw_mgr.GetOverallMetrics();
    
    // Bytes sent/received
    DrawText("Sent: " + FormatBytes(metrics.bytes_sent), impl_->overlay_x, y, impl_->color_text);
    y += impl_->line_height;
    
    DrawText("Recv: " + FormatBytes(metrics.bytes_received), impl_->overlay_x, y, impl_->color_text);
    y += impl_->line_height;
    
    // Packets sent/received
    DrawText("Pkts Sent: " + std::to_string(metrics.packets_sent), impl_->overlay_x, y, impl_->color_text);
    y += impl_->line_height;
    
    DrawText("Pkts Recv: " + std::to_string(metrics.packets_received), impl_->overlay_x, y, impl_->color_text);
    y += impl_->line_height;
    
    // Packet loss
    DrawText("Pkts Lost: " + std::to_string(metrics.packets_lost), impl_->overlay_x, y, impl_->color_text);
    y += impl_->line_height;
    
    // Latency
    std::ostringstream latency_ss;
    latency_ss << "Latency: " << std::fixed << std::setprecision(1) 
               << metrics.average_latency_ms << "ms";
    DrawText(latency_ss.str(), impl_->overlay_x, y, impl_->color_text);
    y += impl_->line_height;
    
    // Bitrate
    std::ostringstream bitrate_ss;
    bitrate_ss << "Bitrate: " << std::fixed << std::setprecision(1) 
               << metrics.current_bitrate_kbps << " kbps";
    DrawText(bitrate_ss.str(), impl_->overlay_x, y, impl_->color_text);
    y += impl_->line_height;
    
    // Coordinator URL
    const auto& config = config_mgr.GetConfig();
    DrawText("Coord: " + config.coordinator.rest_api_url, 
             impl_->overlay_x, y, impl_->color_label);
}

void OverlayRenderer::DrawText(const std::string& text, int x, int y, D3DCOLOR color) {
    if (!impl_->font) {
        return;
    }
    
    RECT rect;
    rect.left = x;
    rect.top = y;
    rect.right = x + 500;  // Wide enough for most text
    rect.bottom = y + impl_->line_height;
    
    impl_->font->DrawTextA(
        nullptr,
        text.c_str(),
        -1,
        &rect,
        DT_LEFT | DT_NOCLIP,
        color
    );
}

std::string OverlayRenderer::FormatBytes(uint64_t bytes) const {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double value = static_cast<double>(bytes);
    
    while (value >= 1024.0 && unit_index < 4) {
        value /= 1024.0;
        unit_index++;
    }
    
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << value << " " << units[unit_index];
    return ss.str();
}

} // namespace P2P