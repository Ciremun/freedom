#pragma once

#include <windows.h>
#include <psapi.h>

#include <stdint.h>

#include "ui/debug_log.h"

void internal_memory_patch(BYTE *dst, BYTE *src, unsigned int size);

template <typename T>
int internal_memory_read(HANDLE hProc, uintptr_t base, T *buffer)
{
    PSAPI_WORKING_SET_EX_INFORMATION info;
    info.VirtualAddress = (PVOID)base;
    if (QueryWorkingSetEx(hProc, &info, sizeof(info)) == 0)
        return 0;
    if (!info.VirtualAttributes.Valid)
        return 0;
    *buffer = *(T *)base;
    return 1;
}
