#include "../../include/NetworkHooks.h"
#include "../../include/Logger.h"
#include <winsock2.h>
#include <ws2tcpip.h>

#include <detours/detours.h>
#include <memory>

namespace P2P {

// Initialize static members
int (WINAPI* NetworkHooks::Original_send)(SOCKET, const char*, int, int) = nullptr;
int (WINAPI* NetworkHooks::Original_sendto)(SOCKET, const char*, int, int, const sockaddr*, int) = nullptr;
int (WINAPI* NetworkHooks::Original_WSASend)(SOCKET, LPWSABUF, DWORD, LPDWORD, DWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE) = nullptr;

// Singleton instance
NetworkHooks& NetworkHooks::GetInstance() {
    static NetworkHooks instance;
    return instance;
}

NetworkHooks::NetworkHooks() : hooks_installed_(false) {
}

NetworkHooks::~NetworkHooks() {
    Shutdown();
}

bool NetworkHooks::Initialize() {
    if (hooks_installed_) {
        return true;
    }

    LOG_INFO("Initializing network hooks...");

    // Load WS2_32.dll to get function addresses
    HMODULE ws2_32 = LoadLibraryA("ws2_32.dll");
    if (!ws2_32) {
        LOG_ERROR("Failed to load ws2_32.dll");
        return false;
    }

    // Get original function addresses
    Original_send = reinterpret_cast<decltype(Original_send)>(GetProcAddress(ws2_32, "send"));
    Original_sendto = reinterpret_cast<decltype(Original_sendto)>(GetProcAddress(ws2_32, "sendto"));
    Original_WSASend = reinterpret_cast<decltype(Original_WSASend)>(GetProcAddress(ws2_32, "WSASend"));

    if (!Original_send || !Original_sendto || !Original_WSASend) {
        LOG_ERROR("Failed to get original function addresses");
        FreeLibrary(ws2_32);
        return false;
    }

    // Install hooks
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    DetourAttach(&(PVOID&)Original_send, Hooked_send);
    DetourAttach(&(PVOID&)Original_sendto, Hooked_sendto);
    DetourAttach(&(PVOID&)Original_WSASend, Hooked_WSASend);

    LONG error = DetourTransactionCommit();
    if (error != NO_ERROR) {
        LOG_ERROR("Failed to install network hooks: " + std::to_string(error));
        FreeLibrary(ws2_32);
        return false;
    }

    hooks_installed_ = true;
    LOG_INFO("Network hooks installed");
    return true;
}

void NetworkHooks::Shutdown() {
    if (!hooks_installed_) {
        return;
    }

    LOG_INFO("Shutting down network hooks...");

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    if (Original_send) DetourDetach(&(PVOID&)Original_send, Hooked_send);
    if (Original_sendto) DetourDetach(&(PVOID&)Original_sendto, Hooked_sendto);
    if (Original_WSASend) DetourDetach(&(PVOID&)Original_WSASend, Hooked_WSASend);

    DetourTransactionCommit();

    hooks_installed_ = false;
    LOG_INFO("Network hooks removed");
}

void NetworkHooks::SetPacketRouter(std::shared_ptr<PacketRouter> router) {
    packet_router_ = router;
}

bool NetworkHooks::IsActive() const {
    return hooks_installed_;
}

int WINAPI NetworkHooks::Hooked_send(SOCKET s, const char* buf, int len, int flags) {
    if (len > 0 && buf) {
        GetInstance().ProcessOutgoingPacket(buf, len);
    }
    return Original_send(s, buf, len, flags);
}

int WINAPI NetworkHooks::Hooked_sendto(SOCKET s, const char* buf, int len, int flags,
                                      const sockaddr* to, int tolen) {
    if (len > 0 && buf) {
        GetInstance().ProcessOutgoingPacket(buf, len);
    }
    return Original_sendto(s, buf, len, flags, to, tolen);
}

int WINAPI NetworkHooks::Hooked_WSASend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount,
                                       LPDWORD lpNumberOfBytesSent, DWORD dwFlags,
                                       LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
    if (dwBufferCount > 0 && lpBuffers) {
        for (DWORD i = 0; i < dwBufferCount; ++i) {
            if (lpBuffers[i].buf && lpBuffers[i].len > 0) {
                GetInstance().ProcessOutgoingPacket(lpBuffers[i].buf, lpBuffers[i].len);
            }
        }
    }
    return Original_WSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags,
                           lpOverlapped, lpCompletionRoutine);
}

bool NetworkHooks::ProcessOutgoingPacket(const char* data, int length) {
    if (!packet_router_) {
        return false;
    }

    try {
        // Parse the packet (RO packets typically have 2-byte header with packet type)
        if (length >= 2) {
            Packet packet;
            packet.type = *reinterpret_cast<const uint16_t*>(data);
            packet.data.assign(data, data + length);
            packet.length = length;

            // Let the packet router decide where to send it
            auto decision = packet_router_->DecideRoute(packet);
            return packet_router_->RoutePacket(packet, decision);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error processing outgoing packet: " + std::string(e.what()));
    }

    return false;
}

Packet NetworkHooks::ParsePacket(const char* data, int length) {
    Packet packet;
    
    if (length >= 2) {
        packet.type = *reinterpret_cast<const uint16_t*>(data);
        packet.data.assign(data, data + length);
        packet.length = length;
    }
    
    return packet;
}

} // namespace P2P