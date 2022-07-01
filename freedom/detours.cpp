#include "detours.h"

twglSwapBuffers wglSwapBuffersGateway;
void_trampoline ar_trampoline;

uintptr_t parse_beatmap_metadata_code_start = 0;
uintptr_t parse_beatmap_metadata_jump_back = 0;

uintptr_t approach_rate_offset_1 = 0;
uintptr_t approach_rate_offset_2 = 0;

Hook SwapBuffersHook;

Hook ApproachRateHook1;
Hook ApproachRateHook2;

static bool find_approach_rate_offsets()
{
    parse_beatmap_metadata_code_start = code_start_for_parse_beatmap_metadata();
    if (!parse_beatmap_metadata_code_start)
        return false;
    const uint8_t approach_rate_1_signature[] = { 0x8B, 0x85, 0xB0, 0xFE, 0xFF, 0xFF, 0xD9, 0x58, 0x2C, 0xEB };
    const uint8_t approach_rate_2_signature[] = { 0x8B, 0x85, 0xB0, 0xFE, 0xFF, 0xFF, 0xD9, 0x58, 0x2C, 0xC7, 0x45, 0xB0, 0x01, 0x00, 0x00, 0x00 };
    for (uintptr_t start = parse_beatmap_metadata_code_start + 0x1000; start - parse_beatmap_metadata_code_start <= 0x1CFF; ++start)
    {
        if (!approach_rate_offset_1 &&
            memcmp((uint8_t *)start, approach_rate_1_signature, sizeof(approach_rate_1_signature)) == 0)
                approach_rate_offset_1 = start - parse_beatmap_metadata_code_start + 0x6;
        if (!approach_rate_offset_2 &&
            memcmp((uint8_t *)start, approach_rate_2_signature, sizeof(approach_rate_2_signature)) == 0)
                approach_rate_offset_2 = start - parse_beatmap_metadata_code_start;
    }
    parse_beatmap_metadata_jump_back = parse_beatmap_metadata_code_start + approach_rate_offset_2 + 0x9;
    return approach_rate_offset_1 && approach_rate_offset_2;
}

bool init_ar_hooks()
{
    if (!find_approach_rate_offsets())
        return false;

    ApproachRateHook1 = Hook((BYTE *)parse_beatmap_metadata_code_start + approach_rate_offset_1, (BYTE *)set_approach_rate_1, (BYTE *)&ar_trampoline, 5);
    ApproachRateHook2 = Hook((BYTE *)parse_beatmap_metadata_code_start + approach_rate_offset_2, (BYTE *)set_approach_rate_2, (BYTE *)&ar_trampoline, 9);

    return true;
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
        mov ebx, cfg_ar_value
        mov dword ptr [eax+0x2C], ebx
        jmp [parse_beatmap_metadata_jump_back]
    }
}

__declspec(naked) void set_approach_rate_2()
{
    __asm {
        mov eax, dword ptr [ebp-0x00000150]
        fstp dword ptr [eax+0x2C]
        mov ebx, cfg_ar_value
        mov dword ptr [eax+0x2C], ebx
        jmp [parse_beatmap_metadata_jump_back]
    }
}
