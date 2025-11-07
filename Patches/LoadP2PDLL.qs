//##################################################################
//# Purpose: Inject P2P Network DLL into Ragnarok Online client   #
//#          at startup for peer-to-peer networking support        #
//##################################################################

function LoadP2PDLL()
{
    // Step 1 - Find the client entry point (WinMain or similar)
    var code = [
        PUSH_R,                    // Save registers
        PUSH, 0x00,                // lpFileName = "p2p_network.dll"
        CALL, [LoadLibraryA],      // Call LoadLibraryA
        TEST, EAX, EAX,            // Check if DLL loaded
        JE, 0x05,                  // Jump if failed
        MOV, [0x00], EAX,          // Store DLL handle
        POP_R                      // Restore registers
    ];
    
    // Step 2 - Find suitable injection point
    // Look for entry point or initialization function
    var offset = exe.findCode(
        " 55"                      // PUSH EBP
      + " 8B EC"                   // MOV EBP, ESP
      + " 6A FF"                   // PUSH -1
      + " 68 AB AB AB AB"          // PUSH offset
      + " 64 A1 00 00 00 00"       // MOV EAX, DWORD PTR FS:[0]
      + " 50"                      // PUSH EAX
    , PTYPE_HEX, true);
    
    if (offset === -1)
    {
        return "Failed to find injection point";
    }
    
    // Step 3 - Allocate space for DLL name string
    var dllName = "p2p_network.dll";
    var dllNameOffset = exe.findZeros(dllName.length + 1);
    
    if (dllNameOffset === -1)
    {
        return "Failed to allocate space for DLL name";
    }
    
    // Step 4 - Write DLL name to allocated space
    exe.replace(dllNameOffset, dllName + "\x00", PTYPE_STRING);
    
    // Step 5 - Get LoadLibraryA address from import table
    var loadLibAddr = exe.findFunction("LoadLibraryA", "KERNEL32.dll");
    
    if (loadLibAddr === -1)
    {
        return "Failed to find LoadLibraryA import";
    }
    
    // Step 6 - Build injection code
    var injectionCode = [
        0x60,                                          // PUSHAD - Save all registers
        0x68, exe.Raw2Rva(dllNameOffset),             // PUSH offset dllName
        0xFF, 0x15, loadLibAddr,                      // CALL DWORD PTR [LoadLibraryA]
        0x85, 0xC0,                                   // TEST EAX, EAX
        0x74, 0x05,                                   // JE short skip_store
        0xA3, exe.Raw2Rva(exe.findZeros(4)),         // MOV [dll_handle], EAX
        0x61,                                          // POPAD - Restore all registers
    ];
    
    // Step 7 - Allocate space for injection code
    var codeOffset = exe.findZeros(injectionCode.length);
    
    if (codeOffset === -1)
    {
        return "Failed to allocate space for injection code";
    }
    
    // Step 8 - Write injection code
    exe.replace(codeOffset, injectionCode, PTYPE_HEX);
    
    // Step 9 - Insert CALL to injection code at entry point
    var callCode = [
        0xE8, exe.Raw2Rva(codeOffset) - exe.Raw2Rva(offset) - 5  // CALL injection_code
    ];
    
    exe.replace(offset, callCode, PTYPE_HEX);
    
    return true;
}

//##################################################################
//# Purpose: Check if patch can be applied                        #
//##################################################################

function LoadP2PDLL_()
{
    // This patch can be applied to all clients
    return (exe.getClientDate() >= 20100000);
}

