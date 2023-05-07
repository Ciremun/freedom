#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>

DWORD get_process_id(const wchar_t* process_name) {
    DWORD process_id = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);

        if (Process32First(hSnap, &procEntry)) {
            do {
                if (!_wcsicmp(procEntry.szExeFile, process_name)) {
                    process_id = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &procEntry));
        }
    }
    CloseHandle(hSnap);
    return process_id;
}

std::vector<uintptr_t> find_all_addresses(HANDLE hProcess, const uintptr_t& base_address, const int& value) {
    std::vector<uintptr_t> addresses;
    uintptr_t offset = 0;
    MEMORY_BASIC_INFORMATION mem_info = { 0 };
    while (VirtualQueryEx(hProcess, (LPCVOID)(base_address + offset), &mem_info, sizeof(mem_info)) == sizeof(mem_info)) {
        if ((mem_info.State & MEM_COMMIT) && (mem_info.Protect & (PAGE_READWRITE | PAGE_WRITECOPY)) && !(mem_info.Protect & PAGE_GUARD)) {
            char* buffer = new char[mem_info.RegionSize];
            SIZE_T bytesRead;
            if (ReadProcessMemory(hProcess, mem_info.BaseAddress, buffer, mem_info.RegionSize, &bytesRead)) {
                for (uintptr_t i = 0; i < bytesRead - sizeof(int); ++i) {
                    if (*(int*)(buffer + i) == value) {
                        addresses.push_back(base_address + offset + i);
                    }
                }
            }
            delete[] buffer;
        }
        offset += mem_info.RegionSize;
    }
    return addresses;
}

int main() {
    wchar_t* process_name = L"osu!.exe";
    DWORD process_id = get_process_id(process_name);

    if (process_id == 0) {
        std::cerr << "Could not find process " << process_name << "!" << std::endl;
        return 1;
    }

    HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, process_id);
    if (hProcess == NULL) {
        std::cerr << "Failed to open process with ID " << process_id << "!" << std::endl;
        return 1;
    }

    MODULEINFO module_info = { 0 };
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule == NULL) {
        std::cerr << "Failed to get handle to current module!" << std::endl;
        CloseHandle(hProcess);
        return 1;
    }
    if (!GetModuleInformation(hProcess, hModule, &module_info, sizeof(module_info))) {
        std::cerr << "Failed to get module information for current module!" << std::endl;
        CloseHandle(hProcess);
        return 1;
    }

    uintptr_t base_address = (uintptr_t)module_info.lpBaseOfDll;
    int value = 0;

    std::vector<uintptr_t> addresses = find_all_addresses(hProcess, base_address, value);
    if (addresses.empty()) {
        std::cerr << "Could not find any matching addresses!" << std::endl;
        CloseHandle
