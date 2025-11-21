#include <windows.h>
#include <iostream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

// Function to inject DLL
bool InjectDLL(HANDLE hProcess, const std::string& dllPath) {
    void* pDllPath = VirtualAllocEx(hProcess, 0, MAX_PATH, MEM_COMMIT, PAGE_READWRITE);
    if (!pDllPath) return false;

    WriteProcessMemory(hProcess, pDllPath, dllPath.c_str(), dllPath.length() + 1, 0);

    HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"), pDllPath, 0, 0);
    if (!hThread) {
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
    return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    char selfPath[MAX_PATH];
    GetModuleFileNameA(NULL, selfPath, MAX_PATH);
    
    std::string selfPathStr = selfPath;
    std::string selfDir = selfPathStr.substr(0, selfPathStr.find_last_of("\\/"));
    std::string selfName = selfPathStr.substr(selfPathStr.find_last_of("\\/") + 1);
    
    // Construct original filename: name_original.exe
    std::string originalName = selfName;
    size_t extPos = originalName.find_last_of(".");
    if (extPos != std::string::npos) {
        originalName.insert(extPos, "_original");
    } else {
        originalName += "_original";
    }
    
    std::string originalPath = selfDir + "\\" + originalName;
    
    if (!fs::exists(originalPath)) {
        MessageBoxA(NULL, ("Could not find original executable:\n" + originalPath).c_str(), "Launcher Error", MB_ICONERROR);
        return 1;
    }

    std::string dllPath = selfDir + "\\p2p_network.dll";
    if (!fs::exists(dllPath)) {
        MessageBoxA(NULL, "p2p_network.dll not found!", "Launcher Error", MB_ICONERROR);
        return 1;
    }

    // Get command line arguments
    std::string cmdLine = GetCommandLineA();
    
    // Create process suspended
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // We need to replace the first token of cmdLine (our path) with the original path
    // But CreateProcess's lpCommandLine can handle it if we just pass the full string? 
    // Better to just pass the original path as lpApplicationName and the full cmdLine.
    // Note: lpCommandLine should include the module name.
    
    // Simple approach: Just pass command line.
    // But we should probably fix the first argument (argv[0]) to be the original exe?
    // Most apps don't care, but let's be safe.
    
    if (!CreateProcessA(originalPath.c_str(), GetCommandLineA(), NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, selfDir.c_str(), &si, &pi)) {
        MessageBoxA(NULL, "Failed to start original game.", "Launcher Error", MB_ICONERROR);
        return 1;
    }

    // Inject DLL
    if (!InjectDLL(pi.hProcess, dllPath)) {
        MessageBoxA(NULL, "Failed to inject p2p_network.dll.", "Launcher Error", MB_ICONERROR);
        TerminateProcess(pi.hProcess, 1);
        return 1;
    }

    // Resume thread
    ResumeThread(pi.hThread);
    
    // Wait for process? No, we can exit.
    // But if we exit, the console might close if we are a console app.
    // We should compile as a GUI app (subsystem:windows) to avoid console window, 
    // or just exit.
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}
