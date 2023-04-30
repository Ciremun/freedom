#pragma once

#include "stdafx.h"

#include <psapi.h>

#include <stdint.h>

#include "log.h"

uintptr_t GetModuleBaseAddress(const wchar_t *modName);

void internal_memory_patch(BYTE *dst, BYTE *src, unsigned int size);
void internal_memory_set(void* dst, int val, unsigned int size);

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

template <size_t N>
uintptr_t internal_multi_level_pointer_dereference(HANDLE hProc, uintptr_t base, const uintptr_t (&offsets)[N])
{
    for (size_t i = 0; i < N; ++i)
    {
        if (internal_memory_read(hProc, base, &base) == 0)
            return 0;
        base += offsets[i];
    }
    return base;
}

template <size_t size>
uintptr_t find_opcodes(const uint8_t (&signature)[size], uintptr_t code_start, int start_offset, int end_offset)
{
    for (uintptr_t start = code_start + start_offset; start - code_start <= end_offset; ++start)
        if (memcmp((uint8_t *)start, signature, size) == 0)
            return start - code_start;
    return 0;
}

template <typename T>
void scan_memory(uintptr_t begin, uintptr_t end, int alignment, T body)
{
    HANDLE hProc = GetCurrentProcess();
    _MEMORY_BASIC_INFORMATION BasicInformation;
    while (VirtualQuery((void *)begin, &BasicInformation, sizeof(BasicInformation)) && begin < end)
    {
        if (BasicInformation.State & MEM_COMMIT)
        {
            unsigned char *block = (unsigned char *)malloc(BasicInformation.RegionSize);
            if (ReadProcessMemory(hProc, (void *)begin, block, BasicInformation.RegionSize, nullptr))
            {
                for (unsigned int idx = 0; idx != BasicInformation.RegionSize / alignment; ++idx)
                {
                    if (body(begin, alignment, block, idx))
                    {
                        free(block);
                        return;
                    }
                }
                free(block);
            }
        }
        begin = (uintptr_t)BasicInformation.BaseAddress + BasicInformation.RegionSize;
    }
}
