#pragma once

#include "utility.h"

#include <windows.h>

#include <stdint.h>

bool detour_32(BYTE *src, BYTE *dst, const uintptr_t len);
BYTE *trampoline_32(BYTE *src, BYTE *dst, const uintptr_t len);

struct Hook
{
    bool bStatus = false;
    BYTE *src = nullptr;
    BYTE *dst = nullptr;
    BYTE *PtrToGatewayFnPtr = nullptr;
    uintptr_t len = 0;

    BYTE originalBytes[10] = {0};

    Hook();
    Hook(BYTE *src, BYTE *dst, BYTE *PtrToGatewayPtr, uintptr_t len);
    Hook(const char *exportName, const char *modName, BYTE *dst, BYTE *PtrToGatewayPtr, uintptr_t len);

    void Enable();
    void Disable();
    void Toggle();
};
