#include "config.h"
#include "detours.h"
#include "hook.h"
#include "dotnet_data_collector.h"

twglSwapBuffers wglSwapBuffersGateway;
void_trampoline ar_trampoline;

uintptr_t parse_beatmap_metadata_code_start = 0;
uintptr_t parse_beatmap_metadata_jump_back = 0;

Hook ApproachRateHook1;
Hook ApproachRateHook2;

void init_hooks()
{
    parse_beatmap_metadata_code_start = code_start_for_parse_beatmap_metadata();
    parse_beatmap_metadata_jump_back = parse_beatmap_metadata_code_start + 0x14C5;

    ApproachRateHook1 = Hook((BYTE *)parse_beatmap_metadata_code_start + 0x146C, (BYTE *)set_approach_rate_1, (BYTE *)&ar_trampoline, 5);
    ApproachRateHook2 = Hook((BYTE *)parse_beatmap_metadata_code_start + 0x14BC, (BYTE *)set_approach_rate_2, (BYTE *)&ar_trampoline, 9);
}

void enable_ar_hooks()
{
    ApproachRateHook1.Enable();
    ApproachRateHook2.Enable();
}

void disable_ar_hooks()
{
    ApproachRateHook1.Disable();
    ApproachRateHook2.Disable();
}

__declspec(naked) void set_approach_rate_1()
{
    __asm {
        fstp dword ptr [eax+0x2C]
        mov ebx, ar_value
        mov dword ptr [eax+0x2C], ebx
        jmp [parse_beatmap_metadata_jump_back]
    }
}

__declspec(naked) void set_approach_rate_2()
{
    __asm {
        mov eax, dword ptr [ebp-0x00000150]
        fstp dword ptr [eax+0x2C]
        mov ebx, ar_value
        mov dword ptr [eax+0x2C], ebx
        jmp [parse_beatmap_metadata_jump_back]
    }
}
