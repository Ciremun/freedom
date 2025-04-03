#include "memory.h"

uintptr_t GetModuleBaseAddress(const wchar_t* modName)
{
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, 0);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry))
        {
            do
            {
                if (!_wcsicmp(modEntry.szModule, modName))
                {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
    CloseHandle(hSnap);
    return modBaseAddr;
}

bool internal_memory_patch(void *dst, void *src, unsigned int size)
{
    DWORD oldprotect;
    if (VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect) == 0)
        return false;
    memcpy(dst, src, size);
    if (VirtualProtect(dst, size, oldprotect, &oldprotect) == 0)
        return false;
    return true;
}
