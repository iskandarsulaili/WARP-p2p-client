#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <detours/detours.h>
#include "Types.h"
#include "PacketRouter.h"
#include <memory>

namespace P2P {

/**
 * NetworkHooks - Hooks into the client's network functions to intercept packets
 */
class NetworkHooks {
public:
    /**
     * Get the global instance (singleton pattern)
     * @return The global NetworkHooks instance
     */
    static NetworkHooks& GetInstance();

    NetworkHooks();
    ~NetworkHooks();

    // Disable copy and move
    NetworkHooks(const NetworkHooks&) = delete;
    NetworkHooks& operator=(const NetworkHooks&) = delete;
    NetworkHooks(NetworkHooks&&) = delete;
    NetworkHooks& operator=(NetworkHooks&&) = delete;

    /**
     * Initialize network hooks
     * @return true if initialization succeeded
     */
    bool Initialize();

    /**
     * Shutdown network hooks
     */
    void Shutdown();

    /**
     * Set the packet router instance
     * @param router The packet router to use
     */
    void SetPacketRouter(std::shared_ptr<PacketRouter> router);

    /**
     * Check if hooks are active
     * @return true if hooks are active
     */
    bool IsActive() const;

private:
    // Original function pointers
    static int (WINAPI* Original_send)(SOCKET s, const char* buf, int len, int flags);
    static int (WINAPI* Original_sendto)(SOCKET s, const char* buf, int len, int flags, 
                                        const sockaddr* to, int tolen);
    static int (WINAPI* Original_WSASend)(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount,
                                         LPDWORD lpNumberOfBytesSent, DWORD dwFlags,
                                         LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

    // Hooked functions
    static int WINAPI Hooked_send(SOCKET s, const char* buf, int len, int flags);
    static int WINAPI Hooked_sendto(SOCKET s, const char* buf, int len, int flags,
                                   const sockaddr* to, int tolen);
    static int WINAPI Hooked_WSASend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount,
                                    LPDWORD lpNumberOfBytesSent, DWORD dwFlags,
                                    LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

    // Helper methods
    bool ProcessOutgoingPacket(const char* data, int length);
    static Packet ParsePacket(const char* data, int length);

    std::shared_ptr<PacketRouter> packet_router_;
    bool hooks_installed_;
};

} // namespace P2P