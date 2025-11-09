#include "../../include/NetworkManager.h"
#include "../../include/ConfigManager.h"
#include "../../include/Logger.h"
#include "../../include/HttpClient.h"
#include "../../include/AuthManager.h"
#include "../../include/SignalingClient.h"
#include "../../include/WebRTCManager.h"
#include "../../include/PacketRouter.h"
#include "../../include/SecurityManager.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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
    std::string current_zone;
};

NetworkManager::NetworkManager() : impl_(std::make_unique<Impl>()) {
    LOG_DEBUG("NetworkManager created");
}

NetworkManager::~NetworkManager() {
    Shutdown();
}

NetworkManager& NetworkManager::GetInstance() {
    static NetworkManager instance;
    return instance;
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

    // Set up WebRTC callbacks for signaling
    impl_->webrtc_manager->SetOnOfferCallback([this](const std::string& peer_id, const std::string& sdp) {
        if (impl_->signaling_client && impl_->signaling_client->IsConnected()) {
            json offer_msg = {
                {"type", "offer"},
                {"to", peer_id},
                {"from", impl_->peer_id},
                {"sdp", sdp}
            };
            impl_->signaling_client->SendMessage(offer_msg.dump());
            LOG_DEBUG("Sent offer to peer: " + peer_id);
        }
    });

    impl_->webrtc_manager->SetOnAnswerCallback([this](const std::string& peer_id, const std::string& sdp) {
        if (impl_->signaling_client && impl_->signaling_client->IsConnected()) {
            json answer_msg = {
                {"type", "answer"},
                {"to", peer_id},
                {"from", impl_->peer_id},
                {"sdp", sdp}
            };
            impl_->signaling_client->SendMessage(answer_msg.dump());
            LOG_DEBUG("Sent answer to peer: " + peer_id);
        }
    });

    impl_->webrtc_manager->SetOnIceCandidateCallback([this](const std::string& peer_id, const std::string& candidate) {
        if (impl_->signaling_client && impl_->signaling_client->IsConnected()) {
            json ice_msg = {
                {"type", "ice_candidate"},
                {"to", peer_id},
                {"from", impl_->peer_id},
                {"candidate", candidate}
            };
            impl_->signaling_client->SendMessage(ice_msg.dump());
            LOG_DEBUG("Sent ICE candidate to peer: " + peer_id);
        }
    });

    // Initialize PacketRouter with proper parameters
    auto server_callback = [](const uint8_t* data, size_t length) -> bool {
        // Suppress unused parameter warning
        (void)data;
        // TODO: Implement actual server routing callback
        LOG_DEBUG("Routing packet to server: " + std::to_string(length) + " bytes");
        return true;
    };

    if (!impl_->packet_router->Initialize(config.GetP2PConfig().enabled,
                                          impl_->webrtc_manager,
                                          server_callback)) {
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
    
    // Authenticate with coordinator (synchronous)
    std::string auth_error;
    if (!impl_->auth_manager->AuthenticateSync(impl_->peer_id, auth_error)) {
        LOG_ERROR("Failed to authenticate: " + auth_error);
        return false;
    }

    LOG_INFO("Authentication successful");
    
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

    impl_->current_zone = zone;

    if (impl_->packet_router) {
        impl_->packet_router->SetCurrentZone(zone);
    }

    // Leave current session if any
    if (!impl_->session_id.empty()) {
        LeaveSession();
    }

    // Check if zone supports P2P
    auto& config = ConfigManager::GetInstance();
    if (config.IsZoneP2PEnabled(zone)) {
        LOG_INFO("P2P enabled for zone: " + zone);

        // Discover and join session
        std::string session_id = DiscoverSession(zone);
        if (!session_id.empty()) {
            if (JoinSession(session_id)) {
                LOG_INFO("Successfully joined P2P session: " + session_id);
            } else {
                LOG_ERROR("Failed to join P2P session: " + session_id);
            }
        } else {
            LOG_WARN("No P2P session available for zone: " + zone);
        }
    } else {
        LOG_INFO("P2P disabled for zone: " + zone);
        // Disconnect from P2P
        if (impl_->signaling_client && impl_->signaling_client->IsConnected()) {
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

std::string NetworkManager::GetCurrentSessionId() const {
    return impl_->session_id;
}

std::string NetworkManager::GetCurrentZone() const {
    return impl_->current_zone;
}

std::string NetworkManager::DiscoverSession(const std::string& zone_id) {
    LOG_INFO("Discovering P2P session for zone: " + zone_id);

    if (!impl_->http_client) {
        LOG_ERROR("HTTP client not initialized");
        return "";
    }

    auto& config = ConfigManager::GetInstance();
    std::string coordinator_url = config.GetCoordinatorUrl();

    // Query coordinator for available sessions in this zone
    std::string url = coordinator_url + "/api/sessions?zone_id=" + zone_id + "&status=active";

    HttpRequest request;
    request.method = "GET";
    request.url = url;
    request.headers["X-API-Key"] = config.GetApiKey();
    request.headers["Authorization"] = "Bearer " + impl_->auth_manager->GetToken();

    HttpResponse response = impl_->http_client->SendRequest(request);

    if (response.status_code != 200) {
        LOG_ERROR("Failed to query sessions: HTTP " + std::to_string(response.status_code));

        // Try to create a new session
        return CreateSession(zone_id);
    }

    try {
        json sessions = json::parse(response.body);

        if (sessions.empty()) {
            LOG_INFO("No active sessions found, creating new session");
            return CreateSession(zone_id);
        }

        // Find session with available slots
        for (const auto& session : sessions) {
            int current_players = session.value("current_players", 0);
            int max_players = session.value("max_players", 50);

            if (current_players < max_players) {
                std::string session_id = session.value("session_id", "");
                LOG_INFO("Found available session: " + session_id +
                        " (" + std::to_string(current_players) + "/" + std::to_string(max_players) + ")");
                return session_id;
            }
        }

        // All sessions full, create new one
        LOG_INFO("All sessions full, creating new session");
        return CreateSession(zone_id);

    } catch (const json::exception& e) {
        LOG_ERROR("Failed to parse session response: " + std::string(e.what()));
        return "";
    }
}

std::string NetworkManager::CreateSession(const std::string& zone_id) {
    LOG_INFO("Creating new P2P session for zone: " + zone_id);

    if (!impl_->http_client) {
        LOG_ERROR("HTTP client not initialized");
        return "";
    }

    auto& config = ConfigManager::GetInstance();
    std::string coordinator_url = config.GetCoordinatorUrl();

    json request_body = {
        {"zone_id", zone_id},
        {"host_id", impl_->peer_id},
        {"max_players", 50}
    };

    HttpRequest request;
    request.method = "POST";
    request.url = coordinator_url + "/api/sessions";
    request.headers["Content-Type"] = "application/json";
    request.headers["X-API-Key"] = config.GetApiKey();
    request.headers["Authorization"] = "Bearer " + impl_->auth_manager->GetToken();
    request.body = request_body.dump();

    HttpResponse response = impl_->http_client->SendRequest(request);

    if (response.status_code != 200 && response.status_code != 201) {
        LOG_ERROR("Failed to create session: HTTP " + std::to_string(response.status_code));
        return "";
    }

    try {
        json session = json::parse(response.body);
        std::string session_id = session.value("session_id", "");
        LOG_INFO("Created new session: " + session_id);
        return session_id;
    } catch (const json::exception& e) {
        LOG_ERROR("Failed to parse create session response: " + std::string(e.what()));
        return "";
    }
}

bool NetworkManager::JoinSession(const std::string& session_id) {
    LOG_INFO("Joining P2P session: " + session_id);

    if (!impl_->signaling_client) {
        LOG_ERROR("Signaling client not initialized");
        return false;
    }

    impl_->session_id = session_id;

    auto& config = ConfigManager::GetInstance();
    std::string signaling_url = config.GetSignalingUrl();

    // Setup signaling callbacks
    impl_->signaling_client->SetOnMessageCallback([this](const std::string& message) {
        HandleSignalingMessage(message);
    });

    impl_->signaling_client->SetOnConnectedCallback([this, session_id]() {
        LOG_INFO("Connected to signaling server");

        // Send join message
        json join_msg = {
            {"type", "join"},
            {"session_id", session_id},
            {"peer_id", impl_->peer_id}
        };

        impl_->signaling_client->SendMessage(join_msg.dump());
    });

    impl_->signaling_client->SetOnDisconnectedCallback([this]() {
        LOG_WARN("Disconnected from signaling server");
        impl_->session_id.clear();
    });

    // Connect to signaling server
    if (!impl_->signaling_client->Connect(signaling_url, impl_->peer_id, session_id)) {
        LOG_ERROR("Failed to connect to signaling server");
        impl_->session_id.clear();
        return false;
    }

    return true;
}

void NetworkManager::LeaveSession() {
    if (impl_->session_id.empty()) {
        return;
    }

    LOG_INFO("Leaving P2P session: " + impl_->session_id);

    // Send leave message
    if (impl_->signaling_client && impl_->signaling_client->IsConnected()) {
        json leave_msg = {
            {"type", "leave"},
            {"session_id", impl_->session_id},
            {"peer_id", impl_->peer_id}
        };

        impl_->signaling_client->SendMessage(leave_msg.dump());
        impl_->signaling_client->Disconnect();
    }

    // Close all peer connections
    if (impl_->webrtc_manager) {
        impl_->webrtc_manager->CloseAllConnections();
    }

    impl_->session_id.clear();
}

void NetworkManager::HandleSignalingMessage(const std::string& message) {
    try {
        json msg = json::parse(message);
        std::string type = msg.value("type", "");

        LOG_DEBUG("Received signaling message: " + type);

        if (type == "offer") {
            // Handle WebRTC offer
            std::string from_peer = msg.value("from", "");
            std::string sdp = msg.value("sdp", "");

            if (impl_->webrtc_manager) {
                impl_->webrtc_manager->HandleOffer(from_peer, sdp);
            }

        } else if (type == "answer") {
            // Handle WebRTC answer
            std::string from_peer = msg.value("from", "");
            std::string sdp = msg.value("sdp", "");

            if (impl_->webrtc_manager) {
                impl_->webrtc_manager->HandleAnswer(from_peer, sdp);
            }

        } else if (type == "ice_candidate") {
            // Handle ICE candidate
            std::string from_peer = msg.value("from", "");
            std::string candidate = msg.value("candidate", "");
            std::string sdp_mid = msg.value("sdpMid", "");
            int sdp_mline_index = msg.value("sdpMLineIndex", 0);

            if (impl_->webrtc_manager) {
                impl_->webrtc_manager->AddIceCandidate(from_peer, candidate, sdp_mid, sdp_mline_index);
            }

        } else if (type == "peer_joined") {
            // New peer joined session
            std::string peer_id = msg.value("peer_id", "");
            LOG_INFO("Peer joined session: " + peer_id);

            // Create offer for new peer
            if (impl_->webrtc_manager && peer_id != impl_->peer_id) {
                impl_->webrtc_manager->CreateOffer(peer_id);
            }

        } else if (type == "peer_left") {
            // Peer left session
            std::string peer_id = msg.value("peer_id", "");
            LOG_INFO("Peer left session: " + peer_id);

            // Close connection to peer
            if (impl_->webrtc_manager) {
                impl_->webrtc_manager->CloseConnection(peer_id);
            }

        } else {
            LOG_WARN("Unknown signaling message type: " + type);
        }

    } catch (const json::exception& e) {
        LOG_ERROR("Failed to parse signaling message: " + std::string(e.what()));
    }
}

} // namespace P2P
