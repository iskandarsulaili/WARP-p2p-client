#include "../../include/NetworkManager.h"
#include "../../include/ConfigManager.h"
#include "../../include/Logger.h"
#include "../../include/HttpClient.h"
#include "../../include/AuthManager.h"
#include "../../include/SignalingClient.h"
#include "../../include/WebRTCManager.h"
#include "../../include/PacketRouter.h"
#include "../../include/NetworkHooks.h"
#include "../../include/SecurityManager.h"
#include "../../include/BandwidthManager.h"
#include "../../include/CompressionManager.h"
#include "../../include/QuicTransport.h"
#include <nlohmann/json.hpp>
#include <ctime>

namespace P2P {

struct NetworkManager::Impl {
    std::shared_ptr<HttpClient> http_client;
    std::shared_ptr<AuthManager> auth_manager;
    std::shared_ptr<SignalingClient> signaling_client;
    std::shared_ptr<WebRTCManager> webrtc_manager;
    std::shared_ptr<PacketRouter> packet_router;
    std::shared_ptr<SecurityManager> security_manager;
    std::shared_ptr<BandwidthManager> bandwidth_manager;
    std::shared_ptr<CompressionManager> compression_manager;
    std::shared_ptr<NetworkHooks> network_hooks;

    // New: QUIC transport
    std::shared_ptr<QuicTransport> quic_transport;

    // Multi-CPU: Host assignment from coordinator
    std::string assigned_host_id;

    bool initialized = false;
    bool active = false;
    std::string peer_id;
    std::string session_id;
    
    // Thread safety: protects all state modifications
    std::mutex mutex;
};

NetworkManager::NetworkManager() : impl_(std::make_unique<Impl>()) {
    LOG_DEBUG("NetworkManager created");
}

NetworkManager::~NetworkManager() noexcept {
    try {
        Shutdown();
    } catch (const std::exception& e) {
        // Cannot propagate exception from destructor
        LOG_ERROR("Exception in NetworkManager destructor: " + std::string(e.what()));
    } catch (...) {
        // Suppress all exceptions in destructor
    }
}

NetworkManager& NetworkManager::GetInstance() {
    static NetworkManager instance;
    return instance;
}

bool NetworkManager::Initialize(const std::string& peer_id) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    
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
    impl_->bandwidth_manager = std::make_shared<BandwidthManager>();
    impl_->compression_manager = std::make_shared<CompressionManager>();

    // Initialize WebRTC
    auto webrtc_config = config.GetWebRTCConfig();
    if (!impl_->webrtc_manager->Initialize(webrtc_config.stun_servers,
                                           webrtc_config.turn_servers,
                                           webrtc_config.turn_username,
                                           webrtc_config.turn_credential,
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

    // Initialize BandwidthManager
    auto bandwidth_config = config.GetBandwidthConfig();
    if (!impl_->bandwidth_manager->Initialize(bandwidth_config)) {
        LOG_ERROR("Failed to initialize BandwidthManager");
        return false;
    }

    // Initialize CompressionManager
    auto compression_config = config.GetCompressionConfig();
    if (!impl_->compression_manager->Initialize(compression_config)) {
        LOG_ERROR("Failed to initialize CompressionManager");
        return false;
    }

    // Initialize NetworkHooks
    impl_->network_hooks = std::make_shared<NetworkHooks>();
    if (!impl_->network_hooks->Initialize()) {
        LOG_ERROR("Failed to initialize NetworkHooks");
        return false;
    }

    // Set packet router for network hooks
    impl_->network_hooks->SetPacketRouter(impl_->packet_router);

    // Set bandwidth manager for packet router
    impl_->packet_router->SetBandwidthManager(impl_->bandwidth_manager.get());

    // Set compression manager for security manager (shared ownership)
    impl_->security_manager->SetCompressionManager(impl_->compression_manager);

    // Transport selection: default to WebRTC, allow QUIC if enabled in config
    bool prefer_quic = config.GetP2PConfig().prefer_quic;
    SelectTransport(prefer_quic);

    impl_->initialized = true;
    LOG_INFO("NetworkManager initialized successfully");

    // Log initial resource usage (CPU/memory) for observability
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus)) {
        LOG_INFO("Initial memory usage: " +
            std::to_string(memStatus.dwMemoryLoad) + "%, Total: " +
            std::to_string(memStatus.ullTotalPhys / (1024 * 1024)) + "MB, Avail: " +
            std::to_string(memStatus.ullAvailPhys / (1024 * 1024)) + "MB");
    }
    // (Optional) Add more resource metrics as needed

    return true;
}

void NetworkManager::SelectTransport(bool prefer_quic) {
    auto& config = ConfigManager::GetInstance();
    if (prefer_quic && config.GetP2PConfig().quic_enabled) {
        // Initialize QUIC transport
        impl_->quic_transport = std::make_shared<QuicTransport>();
        // Example: connect to coordinator's QUIC endpoint (stub)
        std::string quic_address = config.GetCoordinatorConfig().quic_address;
        uint16_t quic_port = config.GetCoordinatorConfig().quic_port;
        if (impl_->quic_transport->Connect(quic_address, quic_port)) {
            LOG_INFO("QUIC transport connected: " + quic_address + ":" + std::to_string(quic_port));
            impl_->packet_router->SetTransport(impl_->quic_transport.get());
            return;
        } else {
            LOG_WARN("Failed to connect QUIC transport, falling back to WebRTC");
        }
    }
    // Default to WebRTCManager as transport
    impl_->packet_router->SetTransport(nullptr); // Use legacy WebRTCManager fallback
    LOG_INFO("Transport set to WebRTCManager (legacy)");
}

ITransport* NetworkManager::GetTransport() const {
    if (impl_->quic_transport && impl_->quic_transport->IsConnected()) {
        return impl_->quic_transport.get();
    }
    // WebRTCManager is used via PacketRouter fallback
    return nullptr;
}

void NetworkManager::Shutdown() {
    if (!impl_->initialized) {
        return;
    }

    Stop();

    if (impl_->network_hooks) impl_->network_hooks->Shutdown();
    if (impl_->security_manager) impl_->security_manager->Shutdown();
    if (impl_->packet_router) impl_->packet_router->Shutdown();
    if (impl_->webrtc_manager) impl_->webrtc_manager->Shutdown();
    if (impl_->signaling_client) impl_->signaling_client->Disconnect();
    if (impl_->auth_manager) impl_->auth_manager->Shutdown();
    if (impl_->quic_transport) impl_->quic_transport->Disconnect();

    impl_->initialized = false;
    LOG_INFO("NetworkManager shutdown complete");
}

bool NetworkManager::Start() {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    
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

    // Log session metrics
    LOG_INFO("Session metrics: peer_id=" + impl_->peer_id +
             ", assigned_host_id=" + impl_->assigned_host_id +
             ", session_id=" + impl_->session_id);

    // Log resource usage at session start
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus)) {
        LOG_INFO("Session start memory usage: " +
            std::to_string(memStatus.dwMemoryLoad) + "%, Total: " +
            std::to_string(memStatus.ullTotalPhys / (1024 * 1024)) + "MB, Avail: " +
            std::to_string(memStatus.ullAvailPhys / (1024 * 1024)) + "MB");
    }

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

    if (impl_->quic_transport) {
        impl_->quic_transport->Disconnect();
    }

    impl_->active = false;
    LOG_INFO("NetworkManager stopped");

    // Log resource usage at session end
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus)) {
        LOG_INFO("Session end memory usage: " +
            std::to_string(memStatus.dwMemoryLoad) + "%, Total: " +
            std::to_string(memStatus.ullTotalPhys / (1024 * 1024)) + "MB, Avail: " +
            std::to_string(memStatus.ullAvailPhys / (1024 * 1024)) + "MB");
    }
}

bool NetworkManager::IsActive() const {
    return impl_->active;
}

BandwidthManager& NetworkManager::GetBandwidthManager() {
    if (!impl_->bandwidth_manager) {
        throw std::runtime_error("BandwidthManager not initialized");
    }
    return *impl_->bandwidth_manager;
}

CompressionManager& NetworkManager::GetCompressionManager() {
    if (!impl_->compression_manager) {
        throw std::runtime_error("CompressionManager not initialized");
    }
    return *impl_->compression_manager;
}

void NetworkManager::OnZoneChange(const std::string& zone) {
    LOG_INFO("Zone changed to: " + zone);

    // Log correlation ID for zone/session traceability
    LOG_INFO("Current correlation ID: " + P2P::Logger::GetInstance().GetCorrelationId());

    if (impl_->packet_router) {
        impl_->packet_router->SetCurrentZone(zone);
    }

    // Check if zone supports P2P
    auto& config = ConfigManager::GetInstance();
    if (config.IsZoneP2PEnabled(zone)) {
        LOG_INFO("P2P enabled for zone: " + zone);
        // Connect to signaling server and join session
        const auto& coordinator_config = config.GetCoordinatorConfig();

        if (impl_->signaling_client) {
            // Generate a unique session ID based on zone and timestamp
            std::string session_id = zone + "_" + std::to_string(std::time(nullptr));

            // Connect to signaling server
            if (impl_->signaling_client->Connect(coordinator_config.websocket_url, impl_->peer_id, session_id)) {
                LOG_INFO("Connected to signaling server for zone: " + zone);

                // Set up message handler for session management
                impl_->signaling_client->SetOnMessageCallback([this](const std::string& message) {
                    HandleSignalingMessage(message);
                });

                // Set up connection/disconnection handlers
                impl_->signaling_client->SetOnConnectedCallback([this]() {
                    LOG_INFO("Signaling connection established");
                    // Send session join request
                    SendSessionRequest();
                });

                impl_->signaling_client->SetOnDisconnectedCallback([this]() {
                    LOG_WARN("Signaling connection lost, switching to server-only mode");
                    if (impl_->packet_router) {
                        impl_->packet_router->EnableP2P(false);
                    }
                    LOG_INFO("Server-only mode enabled due to signaling disconnect");
                });
            } else {
                LOG_ERROR("Failed to connect to signaling server for zone: " + zone);
                if (config.GetZonesConfig().fallback_on_failure) {
                    LOG_INFO("Falling back to server-only mode for zone: " + zone);
                    if (impl_->packet_router) {
                        impl_->packet_router->EnableP2P(false);
                    }
                    LOG_INFO("Server-only mode enabled due to signaling connection failure");
                }
            }
        }
    } else {
        LOG_INFO("P2P disabled for zone: " + zone + ", switching to server-only mode");
        // Disconnect from P2P
        if (impl_->signaling_client) {
            impl_->signaling_client->Disconnect();
        }
        if (impl_->packet_router) {
            impl_->packet_router->EnableP2P(false);
        }
        LOG_INFO("Server-only mode enabled for zone: " + zone);
    }
}

bool NetworkManager::SendPacket(const Packet& packet) {
    if (!impl_->packet_router) {
        LOG_ERROR("SendPacket failed: PacketRouter not initialized");
        return false;
    }

    auto decision = impl_->packet_router->DecideRoute(packet);
    LOG_DEBUG("SendPacket: packet_id=" + std::to_string(packet.packet_id) +
              " type=0x" + std::to_string(packet.type) +
              " length=" + std::to_string(packet.length) +
              " decision=" + std::to_string(static_cast<int>(decision)));

    // Log bandwidth metrics for performance monitoring
    if (impl_->bandwidth_manager) {
        const auto& metrics = impl_->bandwidth_manager->GetOverallMetrics();
        LOG_INFO("Bandwidth: sent=" + std::to_string(metrics.bytes_sent) +
                 "B, recv=" + std::to_string(metrics.bytes_received) +
                 "B, loss=" + std::to_string(metrics.packet_loss_percent) +
                 "%, avg_latency=" + std::to_string(metrics.average_latency_ms) + "ms");
    }

    return impl_->packet_router->RoutePacket(packet, decision);
}

void NetworkManager::HandleSignalingMessage(const std::string& message) {
    LOG_DEBUG("Received signaling message: " + message);

    try {
        auto json_msg = nlohmann::json::parse(message);
        std::string msg_type = json_msg.value("type", "");

        if (msg_type == "session_created") {
            // Session was created successfully
            std::string session_id = json_msg.value("session_id", "");
            LOG_INFO("Session created: " + session_id);

            // Multi-CPU: Parse and log assigned host_id if present
            if (json_msg.contains("host_id")) {
                impl_->assigned_host_id = json_msg.value("host_id", "");
                LOG_INFO("Assigned to host_id (multi-CPU): " + impl_->assigned_host_id);
            } else {
                impl_->assigned_host_id.clear();
                LOG_INFO("No host_id assigned in session (single server or legacy coordinator)");
            }

            // Handle WebRTC offer/answer exchange
            if (json_msg.contains("offer")) {
                // Process WebRTC offer from coordinator
                std::string offer = json_msg["offer"];
                if (impl_->webrtc_manager) {
                    impl_->webrtc_manager->ProcessOffer(offer);
                }
            }

        } else if (msg_type == "peer_joined") {
            // Another peer joined the session
            std::string peer_id = json_msg.value("peer_id", "");
            LOG_INFO("Peer joined: " + peer_id);

        } else if (msg_type == "peer_left") {
            // Peer left the session
            std::string peer_id = json_msg.value("peer_id", "");
            LOG_INFO("Peer left: " + peer_id);

        } else if (msg_type == "ice_candidate") {
            // ICE candidate from another client
            if (impl_->webrtc_manager) {
                impl_->webrtc_manager->AddIceCandidate(message);
            }

        } else if (msg_type == "error") {
            // Error from coordinator
            std::string error_msg = json_msg.value("message", "Unknown error");
            LOG_ERROR("Signaling error: " + error_msg);
        }

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to parse signaling message: " + std::string(e.what()));
    }
}

void NetworkManager::SendSessionRequest() {
    if (!impl_->signaling_client || !impl_->signaling_client->IsConnected()) {
        LOG_WARN("Cannot send session request - not connected to signaling server");
        return;
    }

    try {
        nlohmann::json session_request = {
            {"type", "create_session"},
            {"peer_id", impl_->peer_id},
            {"zone", impl_->packet_router ? impl_->packet_router->GetCurrentZone() : "unknown"},
            {"capabilities", {
                {"webrtc", true},
                {"encryption", true},
                {"compression", false}
            }}
        };

        // Undefine Windows SendMessage macro to avoid conflict
        #ifdef SendMessage
        #undef SendMessage
        #endif

        if (impl_->signaling_client->SendMessage(session_request.dump())) {
            LOG_INFO("Session request sent");
        } else {
            LOG_ERROR("Failed to send session request");
        }

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create session request: " + std::string(e.what()));
    }
}

} // namespace P2P