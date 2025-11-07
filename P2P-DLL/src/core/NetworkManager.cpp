#include "../../include/NetworkManager.h"
#include "../../include/ConfigManager.h"
#include "../../include/Logger.h"
#include "../../include/HttpClient.h"
#include "../../include/AuthManager.h"
#include "../../include/SignalingClient.h"
#include "../../include/WebRTCManager.h"
#include "../../include/PacketRouter.h"
#include "../../include/SecurityManager.h"

namespace P2P {

struct NetworkManager::Impl {
    std::shared_ptr<HttpClient> http_client;
    std::shared_ptr<AuthManager> auth_manager;
    std::shared_ptr<SignalingClient> signaling_client;
    std::shared_ptr<WebRTCManager> webrtc_manager;
    std::shared_ptr<PacketRouter> packet_router;
    std::shared_ptr<SecurityManager> security_manager;
    
    bool initialized = false;
    bool active = false;
    std::string peer_id;
    std::string session_id;
};

NetworkManager::NetworkManager() : impl_(std::make_unique<Impl>()) {
    LOG_DEBUG("NetworkManager created");
}

NetworkManager::~NetworkManager() {
    Shutdown();
}

bool NetworkManager::Initialize(const std::string& peer_id) {
    if (impl_->initialized) {
        LOG_WARN("NetworkManager already initialized");
        return true;
    }
    
    impl_->peer_id = peer_id;
    
    // Load configuration
    auto& config = ConfigManager::GetInstance();
    if (!config.LoadFromFile("config/p2p_config.json")) {
        LOG_ERROR("Failed to load configuration");
        return false;
    }
    
    // Check if P2P is enabled
    if (!config.GetP2PConfig().enabled) {
        LOG_INFO("P2P is disabled in configuration");
        impl_->initialized = true;
        return true;
    }
    
    // Initialize components
    impl_->http_client = std::make_shared<HttpClient>();
    impl_->http_client->SetBaseUrl(config.GetCoordinatorConfig().rest_api_url);
    impl_->http_client->SetTimeout(config.GetCoordinatorConfig().timeout_ms);
    
    impl_->auth_manager = std::make_shared<AuthManager>();
    if (!impl_->auth_manager->Initialize(impl_->http_client, config.GetCoordinatorConfig().rest_api_url)) {
        LOG_ERROR("Failed to initialize AuthManager");
        return false;
    }
    
    impl_->signaling_client = std::make_shared<SignalingClient>();
    impl_->webrtc_manager = std::make_shared<WebRTCManager>();
    impl_->packet_router = std::make_shared<PacketRouter>();
    impl_->security_manager = std::make_shared<SecurityManager>();
    
    // Initialize WebRTC
    auto webrtc_config = config.GetWebRTCConfig();
    if (!impl_->webrtc_manager->Initialize(webrtc_config.stun_servers, 
                                           webrtc_config.turn_servers,
                                           config.GetP2PConfig().max_peers)) {
        LOG_ERROR("Failed to initialize WebRTCManager");
        return false;
    }
    
    // Initialize PacketRouter
    if (!impl_->packet_router->Initialize(config.GetP2PConfig().enabled)) {
        LOG_ERROR("Failed to initialize PacketRouter");
        return false;
    }
    
    // Initialize SecurityManager
    if (!impl_->security_manager->Initialize(config.GetSecurityConfig().encryption_enabled)) {
        LOG_ERROR("Failed to initialize SecurityManager");
        return false;
    }
    
    impl_->initialized = true;
    LOG_INFO("NetworkManager initialized successfully");
    return true;
}

void NetworkManager::Shutdown() {
    if (!impl_->initialized) {
        return;
    }
    
    Stop();
    
    if (impl_->security_manager) impl_->security_manager->Shutdown();
    if (impl_->packet_router) impl_->packet_router->Shutdown();
    if (impl_->webrtc_manager) impl_->webrtc_manager->Shutdown();
    if (impl_->signaling_client) impl_->signaling_client->Disconnect();
    if (impl_->auth_manager) impl_->auth_manager->Shutdown();
    
    impl_->initialized = false;
    LOG_INFO("NetworkManager shutdown complete");
}

bool NetworkManager::Start() {
    if (!impl_->initialized) {
        LOG_ERROR("NetworkManager not initialized");
        return false;
    }
    
    if (impl_->active) {
        LOG_WARN("NetworkManager already active");
        return true;
    }
    
    // Authenticate with coordinator
    bool auth_success = false;
    impl_->auth_manager->Authenticate(impl_->peer_id, [&](bool success, const std::string& error) {
        if (success) {
            LOG_INFO("Authentication successful");
            auth_success = true;
        } else {
            LOG_ERROR("Authentication failed: " + error);
        }
    });
    
    if (!auth_success) {
        LOG_ERROR("Failed to authenticate");
        return false;
    }
    
    // Start auto-refresh
    impl_->auth_manager->StartAutoRefresh(3600);
    
    impl_->active = true;
    LOG_INFO("NetworkManager started");
    return true;
}

void NetworkManager::Stop() {
    if (!impl_->active) {
        return;
    }
    
    if (impl_->auth_manager) {
        impl_->auth_manager->StopAutoRefresh();
    }
    
    if (impl_->signaling_client) {
        impl_->signaling_client->Disconnect();
    }
    
    impl_->active = false;
    LOG_INFO("NetworkManager stopped");
}

bool NetworkManager::IsActive() const {
    return impl_->active;
}

void NetworkManager::OnZoneChange(const std::string& zone) {
    LOG_INFO("Zone changed to: " + zone);
    
    if (impl_->packet_router) {
        impl_->packet_router->SetCurrentZone(zone);
    }
    
    // Check if zone supports P2P
    auto& config = ConfigManager::GetInstance();
    if (config.IsZoneP2PEnabled(zone)) {
        LOG_INFO("P2P enabled for zone: " + zone);
        // TODO: Connect to signaling server and join session
    } else {
        LOG_INFO("P2P disabled for zone: " + zone);
        // Disconnect from P2P
        if (impl_->signaling_client) {
            impl_->signaling_client->Disconnect();
        }
    }
}

bool NetworkManager::SendPacket(const Packet& packet) {
    if (!impl_->packet_router) {
        return false;
    }
    
    auto decision = impl_->packet_router->DecideRoute(packet);
    return impl_->packet_router->RoutePacket(packet, decision);
}

} // namespace P2P
