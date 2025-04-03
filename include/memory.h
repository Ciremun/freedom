#pragma once

#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

#include <stdint.h>
#include <inttypes.h>

uintptr_t GetModuleBaseAddress(const wchar_t* modName);
bool internal_memory_patch(void *dst, void *src, unsigned int size);

template <typename T>
bool internal_memory_read(HANDLE hProc, uintptr_t base, T *buffer)
{
    PSAPI_WORKING_SET_EX_INFORMATION info;
    info.VirtualAddress = (PVOID)base;
    if (QueryWorkingSetEx(hProc, &info, sizeof(info)) == 0)
        return false;
    if (!info.VirtualAttributes.Valid)
        return false;
    *buffer = *(T *)base;
    return true;
}
