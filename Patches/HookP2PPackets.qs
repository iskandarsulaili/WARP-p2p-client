//
// P2P Packet Hooking Patch
// Hooks send/recv functions to intercept game packets for P2P routing
//
// This patch intercepts Winsock send() and recv() calls to route packets
// through the P2P DLL for peer-to-peer gameplay
//

function HookP2PPackets()
{
    // Find send() and recv() import calls
    var sendFunc = imports.ptrValidated("send", "ws2_32.dll");
    var recvFunc = imports.ptrValidated("recv", "ws2_32.dll");
    
    if (sendFunc === -1 || recvFunc === -1)
    {
        throw "send() or recv() function not found in imports";
    }
    
    // Allocate space for hook functions
    var hookSize = 256;
    var sendHookAddr = pe.findFree(hookSize);
    var recvHookAddr = pe.findFree(hookSize);
    
    if (sendHookAddr === -1 || recvHookAddr === -1)
    {
        throw "Not enough free space for hook functions";
    }
    
    // Get P2P DLL function addresses (these will be resolved at runtime)
    // The P2P DLL exports: P2P_RoutePacket(socket, buffer, length, flags, isSend)
    var p2pDllName = "P2P-Network.dll";
    var p2pFuncName = "P2P_RoutePacket";
    
    // Create send() hook
    var sendHookCode = 
        // Save registers
        "60 " +                                    // pushad
        "9C " +                                    // pushfd
        
        // Call P2P_RoutePacket(socket, buffer, length, flags, 1)
        "6A 01 " +                                 // push 1 (isSend = true)
        "FF 74 24 2C " +                           // push [esp+2C] (flags)
        "FF 74 24 2C " +                           // push [esp+2C] (length)
        "FF 74 24 2C " +                           // push [esp+2C] (buffer)
        "FF 74 24 2C " +                           // push [esp+2C] (socket)
        "B8 " + "00 00 00 00 " +                   // mov eax, P2P_RoutePacket (placeholder)
        "FF D0 " +                                 // call eax
        "83 C4 14 " +                              // add esp, 14 (clean up 5 params)
        
        // Check return value (0 = route to server, 1 = handled by P2P)
        "85 C0 " +                                 // test eax, eax
        "75 0A " +                                 // jnz skip_original_send
        
        // Restore and call original send()
        "9D " +                                    // popfd
        "61 " +                                    // popad
        "FF 25 " + sendFunc.packToHex(4) +         // jmp [send]
        
        // skip_original_send: P2P handled it, return success
        "9D " +                                    // popfd
        "61 " +                                    // popad
        "8B 44 24 0C " +                           // mov eax, [esp+0C] (return length)
        "C2 10 00";                                // ret 10
    
    // Create recv() hook
    var recvHookCode =
        // Save registers
        "60 " +                                    // pushad
        "9C " +                                    // pushfd
        
        // Call P2P_RoutePacket(socket, buffer, length, flags, 0)
        "6A 00 " +                                 // push 0 (isSend = false)
        "FF 74 24 2C " +                           // push [esp+2C] (flags)
        "FF 74 24 2C " +                           // push [esp+2C] (length)
        "FF 74 24 2C " +                           // push [esp+2C] (buffer)
        "FF 74 24 2C " +                           // push [esp+2C] (socket)
        "B8 " + "00 00 00 00 " +                   // mov eax, P2P_RoutePacket (placeholder)
        "FF D0 " +                                 // call eax
        "83 C4 14 " +                              // add esp, 14 (clean up 5 params)
        
        // Check return value
        "85 C0 " +                                 // test eax, eax
        "75 0A " +                                 // jnz skip_original_recv
        
        // Restore and call original recv()
        "9D " +                                    // popfd
        "61 " +                                    // popad
        "FF 25 " + recvFunc.packToHex(4) +         // jmp [recv]
        
        // skip_original_recv: P2P handled it
        "9D " +                                    // popfd
        "61 " +                                    // popad
        "8B 44 24 0C " +                           // mov eax, [esp+0C] (return length)
        "C2 10 00";                                // ret 10
    
    // Insert hook code
    pe.insertHexAt(sendHookAddr, hookSize, sendHookCode);
    pe.insertHexAt(recvHookAddr, hookSize, recvHookCode);
    
    // Replace all send() calls with our hook
    var sendOffsets = pe.findCodes("FF 15 " + sendFunc.packToHex(4));
    for (var i = 0; i < sendOffsets.length; i++)
    {
        pe.replaceDWord(sendOffsets[i] + 2, pe.rawToVa(sendHookAddr));
    }
    
    // Replace all recv() calls with our hook
    var recvOffsets = pe.findCodes("FF 15 " + recvFunc.packToHex(4));
    for (var i = 0; i < recvOffsets.length; i++)
    {
        pe.replaceDWord(recvOffsets[i] + 2, pe.rawToVa(recvHookAddr));
    }
    
    return true;
}

function HookP2PPackets_()
{
    // This patch requires the LoadP2PDLL patch to be applied first
    return pe.getDate() >= 20100101; // Works with all modern clients
}

