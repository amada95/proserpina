#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

DWORD GetProcID(const char *procName) {
    DWORD procID = 0;
    HANDLE snapHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapHandle != INVALID_HANDLE_VALUE) {

        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);

        if (Process32First(snapHandle, &procEntry)) {
            do {
                if (!_stricmp(procEntry.szExeFile, procName)) {
                    procID = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(snapHandle, &procEntry));
        }
    }

    CloseHandle(snapHandle);
    return procID;
}

int InjectDLL(const char *dllPath, const char *procName) {
    DWORD procID = 0;
    while (!procID) procID = GetProcID(procName);

    HANDLE procHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, procID);

    if (procHandle && procHandle != INVALID_HANDLE_VALUE) {
        void* location = VirtualAllocEx(procHandle, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

        if (location) {
            WriteProcessMemory(procHandle, location, dllPath, strlen(dllPath) + 1, 0);

            HANDLE threadHandle = CreateRemoteThread(procHandle, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, location, 0, 0);
            if (threadHandle) CloseHandle(threadHandle);
        }
        if (procHandle) CloseHandle(procHandle);
    } else return 1;

    return 0;
}

int main (int argc, char *argv[]) {
    const char* dllPath  = argv[1];
    const char* procName = argv[2];
    
    return InjectDLL(dllPath, procName);
}
