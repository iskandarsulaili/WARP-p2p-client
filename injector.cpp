#include <windows.h>
#include <iostream>
#include <string>
#include <tlhelp32.h>

DWORD GetProcessIdByName(const std::wstring& processName) {
    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;

    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (processName == entry.szExeFile) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return 0;
}

int main() {
    const std::wstring targetProcess = L"2025-06-04_Speedrun.exe";
    const std::string dllName = "p2p_network.dll";
    char fullDllPath[MAX_PATH];
    GetFullPathNameA(dllName.c_str(), MAX_PATH, fullDllPath, NULL);

    std::cout << "Waiting for " << std::string(targetProcess.begin(), targetProcess.end()) << "..." << std::endl;

    DWORD pid = 0;
    while ((pid = GetProcessIdByName(targetProcess)) == 0) {
        Sleep(1000);
    }

    std::cout << "Found PID: " << pid << std::endl;
    
    // Wait a bit for initialization
    Sleep(2000);

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        std::cerr << "Failed to open process." << std::endl;
        return 1;
    }

    void* pDllPath = VirtualAllocEx(hProcess, 0, MAX_PATH, MEM_COMMIT, PAGE_READWRITE);
    WriteProcessMemory(hProcess, pDllPath, fullDllPath, strlen(fullDllPath) + 1, 0);

    HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"), pDllPath, 0, 0);
    
    if (hThread) {
        std::cout << "Injected " << fullDllPath << std::endl;
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    } else {
        std::cerr << "Failed to create remote thread." << std::endl;
    }

    VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    return 0;
}
