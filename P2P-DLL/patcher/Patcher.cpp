#include <windows.h>
#include <detours.h>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#pragma comment(lib, "detours.lib")

namespace fs = std::filesystem;

// Callback to list/preserve existing imports
static BOOL CALLBACK ListBywayCallback(PVOID pContext, PCHAR pszFile, PCHAR *ppszOutFile) {
    (void)pContext;
    *ppszOutFile = pszFile;
    return TRUE;
}

static BOOL CALLBACK ListFileCallback(PVOID pContext, PCHAR pszOrigFile, PCHAR pszFile, PCHAR *ppszOutFile) {
    (void)pContext;
    *ppszOutFile = pszFile;
    return TRUE;
}

static BOOL CALLBACK ListSymbolCallback(PVOID pContext, ULONG nOrigOrdinal, ULONG nOrdinal, ULONG *pnOutOrdinal, PCHAR pszOrigSymbol, PCHAR pszSymbol, PCHAR *ppszOutSymbol) {
    (void)pContext;
    *pnOutOrdinal = nOrdinal;
    *ppszOutSymbol = pszSymbol;
    return TRUE;
}

bool PatchExecutable(const std::string& targetPath, const std::string& dllName) {
    std::cout << "Patching " << targetPath << " with " << dllName << "..." << std::endl;

    // Open the file
    HANDLE hFile = CreateFileA(targetPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: Could not open file " << targetPath << std::endl;
        return false;
    }

    PDETOUR_BINARY pBinary = DetourBinaryOpen(hFile);
    if (pBinary == NULL) {
        std::cerr << "Error: DetourBinaryOpen failed." << std::endl;
        CloseHandle(hFile);
        return false;
    }

    HANDLE hNewFile = INVALID_HANDLE_VALUE;
    PDETOUR_BINARY pNewBinary = NULL;
    std::string tempPath = targetPath + ".tmp";

    hNewFile = CreateFileA(tempPath.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (hNewFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: Could not create temp file " << tempPath << std::endl;
        DetourBinaryClose(pBinary);
        CloseHandle(hFile);
        return false;
    }

    pNewBinary = DetourBinaryOpen(hNewFile);
    if (pNewBinary == NULL) {
        std::cerr << "Error: DetourBinaryOpen (new) failed." << std::endl;
        CloseHandle(hNewFile);
        DetourBinaryClose(pBinary);
        CloseHandle(hFile);
        return false;
    }

    // Copy existing imports
    if (!DetourBinaryEditImports(pBinary, pNewBinary, ListBywayCallback, ListFileCallback, ListSymbolCallback, NULL)) {
        std::cerr << "Error: DetourBinaryEditImports failed." << std::endl;
        DetourBinaryClose(pNewBinary);
        CloseHandle(hNewFile);
        DetourBinaryClose(pBinary);
        CloseHandle(hFile);
        return false;
    }

    // Add the new DLL
    if (!DetourBinaryEditImports(pBinary, pNewBinary, NULL, NULL, NULL, NULL)) { // Reset? No, we need to add.
         // Actually, to ADD a payload, we use DetourBinaryEditImports with a callback that adds it?
         // No, Detours has a specific way to add a payload.
         // Wait, DetourBinaryEditImports is for editing the Import Table.
         // To add a DLL, we usually just need to ensure it's in the list.
    }
    
    // Correct usage for ADDING a DLL:
    // We need to implement a callback that adds our DLL if it's not there?
    // Or use DetourBinarySetPayload? No, that's for data payloads.
    
    // Let's look at setdll.cpp logic.
    // It uses DetourBinaryEditImports(pBinary, pNewBinary, ... , AddFileCallback, ... )
    
    // We need to close everything first to implement the logic correctly in the callback.
    // But wait, I can't define the callback inside the function easily without context.
    
    // Let's restart the logic with the correct callback approach.
    return false; 
}

// We need a context to pass the DLL name
struct PatchContext {
    const char* dllName;
    bool added;
};

static BOOL CALLBACK AddFileCallback(PVOID pContext, PCHAR pszOrigFile, PCHAR pszFile, PCHAR *ppszOutFile) {
    PatchContext* ctx = (PatchContext*)pContext;
    
    // Preserve existing
    *ppszOutFile = pszFile;
    
    // Check if we already added it (or if it's already there)
    if (pszFile && _stricmp(pszFile, ctx->dllName) == 0) {
        ctx->added = true;
    }
    
    return TRUE;
}

static BOOL CALLBACK AddBywayCallback(PVOID pContext, PCHAR pszFile, PCHAR *ppszOutFile) {
    (void)pContext;
    *ppszOutFile = pszFile;
    return TRUE;
}

static BOOL CALLBACK AddSymbolCallback(PVOID pContext, ULONG nOrigOrdinal, ULONG nOrdinal, ULONG *pnOutOrdinal, PCHAR pszOrigSymbol, PCHAR pszSymbol, PCHAR *ppszOutSymbol) {
    (void)pContext;
    *pnOutOrdinal = nOrdinal;
    *ppszOutSymbol = pszSymbol;
    return TRUE;
}

// The magic happens here: Detours doesn't have a simple "AddDll" function.
// We have to use DetourBinaryEditImports.
// But wait, setdll.cpp uses `DetourBinaryEditImports` to COPY existing imports.
// To ADD one, we have to insert it into the stream?
// Actually, Detours documentation says:
// "To add an import, the user must provide a callback function that lists the new import."
// But the callback iterates over EXISTING imports.
// Ah, if we want to add one, we might need to use `DetourBinaryEditImports` and then...
// Wait, `setdll.cpp` works by:
// 1. Listing all existing imports.
// 2. It seems it DOESN'T use DetourBinaryEditImports to add.
// It uses `DetourBinaryEditImports` to rewrite the import table.
// AND it passes a context.
// If the callback is called with NULL, it implies end of list?
// No.

// Let's check `setdll.cpp` source code pattern.
// It defines `DetourBinaryEditImports(..., Byway, File, Symbol, ...)`
// Inside `File` callback:
// It just returns the file.
// BUT, where does it add the new one?
// Ah! `DetourBinaryEditImports` takes `pContext`.
// But how does it inject a NEW file?
//
// Found it: You have to call `DetourBinaryEditImports` and in the `File` callback, you can decide to output MORE files?
// No, the callback signature is 1-to-1.
//
// Actually, `setdll` does this:
// It uses `DetourBinaryEditImports` to copy.
// AND it uses `DetourBinarySetPayload` to add a payload? No, that's for payloads.
//
// Wait, `setdll` ADDS the DLL to the Import Table.
// How?
// "DetourBinaryEditImports ... The callback functions are called for each import ... "
//
// Maybe I should use `LIEF` again but CORRECTLY?
// The user said "patch the .exe".
// LIEF is capable. My previous error was `AttributeError`.
// And then `0xc0000142`.
// `0xc0000142` with LIEF often means the IAT was rebuilt but the OEP or something else wasn't updated, or the new Import Table overlaps with something.
// LIEF's `builder.build_imports(True)` should handle this by creating a new section.
//
// Let's stick to Detours because it's Microsoft's own library for this and usually safer for Windows binaries.
//
// Re-reading Detours `setdll` implementation:
// It uses `DetourBinaryEditImports`.
// But it seems I missed how it adds the new DLL.
//
// Ah! `DetourBinaryEditImports` has a feature:
// If you want to ADD a DLL, you can't just use the callback.
// You have to use `DetourBinaryEditImports` to copy existing, AND...
//
// Actually, `setdll` uses `DetourBinaryEditImports` and passes a callback that *modifies* the list?
//
// Let's look at a simpler way: `DetourCreateProcessWithDll`. That's for runtime.
//
// Okay, let's look at `LIEF` C++ implementation.
// `binary->add_library("p2p_network.dll");`
// `binary->write("patched.exe");`
// This is what I did in Python.
//
// Maybe the issue is `p2p_network.dll` itself?
// No, the injector worked.
//
// Let's try to build the Patcher using `LIEF` (C++) if available, OR `Detours` if I can figure out the "Add" part.
//
// Wait, `setdll` uses `DetourBinaryEditImports` to *remove* or *replace*.
// To ADD, it might be tricky.
//
// Let's go back to LIEF (Python) and fix the `0xc0000142`.
// Why did it fail?
// Maybe `p2p_network.dll` needs to be the *first* import?
// Or maybe the section created by LIEF is not executable/readable?
//
// Let's try to create a C++ Patcher that uses `LIEF` (since vcpkg installed it).
// `find_package(LIEF CONFIG REQUIRED)`
//
// Code:
// #include <LIEF/LIEF.hpp>
// int main() {
//   auto binary = LIEF::PE::Parser::parse("ragexe.exe");
//   binary->add_library("p2p_network.dll");
//   binary->write("ragexe_patched.exe");
// }
//
// This is exactly what the Python script did.
// If Python failed, C++ might fail too (same logic).
//
// What if I use `Editbin`?
// No.
//
// Let's look at the `setdll` source code again (I can't browse the web, but I can recall or guess).
// Actually, `setdll` DOES use `DetourBinaryEditImports`.
// And it passes a list of DLLs to add.
//
// Wait, I can use `vcpkg` to install `detours` and check the headers.
// `d:\RO\patcher\WARP-p2p-client\P2P-DLL\vcpkg_installed\x86-windows\include\detours.h`
//
// Let's write a script to inspect `detours.h` to see the API.
//
// Also, I can try to find `setdll.exe` in the vcpkg installed tools?
// `dir .../tools/detours`?
//
// If `setdll.exe` exists, I can just copy it and rename it `Patcher.exe` and wrap it in a batch file?
// User said "No need .bat".
//
// Okay, I will write a C++ program that invokes `setdll` logic.
//
// Let's check if `setdll.exe` exists.
