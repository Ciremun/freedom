#pragma once

#include "stdafx.h"

#include <psapi.h>

#include <stdint.h>

#ifndef NDEBUG
#define FR_ERROR(message) fprintf(stderr, "ERROR: %s:%d: %s\n", __FUNCSIG__, __LINE__, message)
#define FR_ERROR_FMT(fmt, ...) fprintf(stderr, "ERROR: %s:%d: " fmt "\n", __FUNCSIG__, __LINE__, __VA_ARGS__)
#define FR_INFO(message) fprintf(stdout, "%s\n", message)
#define FR_INFO_FMT(fmt, ...) fprintf(stdout, fmt "\n", __VA_ARGS__)
#else
#define FR_ERROR(message)
#define FR_ERROR_FMT(fmt, ...)
#define FR_INFO(message)
#define FR_INFO_FMT(fmt, ...)
#endif // NDEBUG

#define FR_PTR_INFO(...) FR_INFO_FMT("%-35.35s 0x%X", __VA_ARGS__)

uintptr_t GetModuleBaseAddress(const wchar_t *modName);

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
