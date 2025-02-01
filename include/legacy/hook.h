#pragma once

#include "memory.h"

#include <windows.h>

#include <stdint.h>
#include <assert.h>

bool detour_32(BYTE *src, BYTE *dst, const uintptr_t len);
BYTE *trampoline_32(BYTE *src, BYTE *dst, const uintptr_t len);

struct Detour32 {};
struct Trampoline32 {};

template <typename T>
struct Hook
{
    BYTE *src = 0;
    BYTE *dst = 0;
    BYTE *PtrToGatewayFnPtr = 0;
    uintptr_t len = 0;
    BYTE originalBytes[16] = {0};
    bool enabled = false;
    bool free_gateway = true;

    Hook() {}
    Hook(uintptr_t src, BYTE *dst, uintptr_t len) : src((BYTE *)src), dst(dst), len(len) {}
    Hook(BYTE *src, BYTE *dst, uintptr_t len) : src(src), dst(dst), len(len) {}
    Hook(BYTE *src, BYTE *dst, BYTE *PtrToGatewayFnPtr, uintptr_t len) : src(src), dst(dst), len(len), PtrToGatewayFnPtr(PtrToGatewayFnPtr) {}
    Hook(uintptr_t src, BYTE *dst, BYTE *PtrToGatewayFnPtr, uintptr_t len) : src((BYTE *)src), dst(dst), len(len), PtrToGatewayFnPtr(PtrToGatewayFnPtr) {}
    Hook(const char *exportName, const char *modName, BYTE *dst, BYTE *PtrToGatewayFnPtr, uintptr_t len) : dst(dst), len(len), PtrToGatewayFnPtr(PtrToGatewayFnPtr)
    {
        HMODULE hMod = GetModuleHandleA(modName);
        this->src = (BYTE *)GetProcAddress(hMod, exportName);
    }
    void Enable();
    void Disable();
};

template <>
inline void Hook<Trampoline32>::Enable()
{
    assert(len <= 16);
    if (!enabled && len)
    {
        memcpy(originalBytes, src, len);
        *(uintptr_t *)PtrToGatewayFnPtr = (uintptr_t)trampoline_32(src, dst, len);
        enabled = true;
    }
}

template <>
inline void Hook<Trampoline32>::Disable()
{
    if (enabled)
    {
        internal_memory_patch(src, originalBytes, len);
        if (free_gateway)
            VirtualFree(*(LPVOID *)PtrToGatewayFnPtr, 0, MEM_RELEASE);
        enabled = false;
    }
}

template <>
inline void Hook<Detour32>::Enable()
{
    assert(len <= 16);
    if (!enabled && len)
    {
        memcpy(originalBytes, src, len);
        detour_32(src, dst, len);
        enabled = true;
    }
}


template <>
inline void Hook<Detour32>::Disable()
{
    if (enabled)
    {
        internal_memory_patch(src, originalBytes, len);
        enabled = false;
    }
}
