#include "detours.h"

twglSwapBuffers wglSwapBuffersGateway;
void_trampoline empty_trampoline;

bool ar_offsets_found = false;
bool cs_offsets_found = false;

uintptr_t parse_beatmap_metadata_code_start = 0;

uintptr_t approach_rate_offset_1 = 0;
uintptr_t approach_rate_offset_2 = 0;
uintptr_t ar_hook_jump_back = 0;

uintptr_t circle_size_offsets[3] = {0};
uintptr_t cs_hook_jump_back = 0;

Hook SwapBuffersHook;

Hook ApproachRateHook1;
Hook ApproachRateHook2;

Hook CircleSizeHook_1;
Hook CircleSizeHook_2;
Hook CircleSizeHook_3;

void try_find_hook_offsets()
{
    parse_beatmap_metadata_code_start = code_start_for_parse_beatmap_metadata();
    if (!parse_beatmap_metadata_code_start)
        return;
    const uint8_t approach_rate_1_signature[] = { 0x8B, 0x85, 0xB0, 0xFE, 0xFF, 0xFF, 0xD9, 0x58, 0x2C, 0xEB };
    const uint8_t approach_rate_2_signature[] = { 0x8B, 0x85, 0xB0, 0xFE, 0xFF, 0xFF, 0xD9, 0x58, 0x2C, 0xC7, 0x45, 0xB0, 0x01, 0x00, 0x00, 0x00 };
    const uint8_t circle_size_signature[]     = { 0x8B, 0x85, 0xB0, 0xFE, 0xFF, 0xFF, 0xD9, 0x58, 0x30, 0xE9 };
    int circle_size_offsets_idx = 0;

    for (uintptr_t start = parse_beatmap_metadata_code_start + 0x1000; start - parse_beatmap_metadata_code_start <= 0x1CFF; ++start)
    {
        if (!approach_rate_offset_1 &&
            memcmp((uint8_t *)start, approach_rate_1_signature, sizeof(approach_rate_1_signature)) == 0)
                approach_rate_offset_1 = start - parse_beatmap_metadata_code_start + 0x6;
        if (!approach_rate_offset_2 &&
            memcmp((uint8_t *)start, approach_rate_2_signature, sizeof(approach_rate_2_signature)) == 0)
                approach_rate_offset_2 = start - parse_beatmap_metadata_code_start;
        if (circle_size_offsets_idx != 3 &&
            memcmp((uint8_t *)start, circle_size_signature, sizeof(circle_size_signature)) == 0)
                circle_size_offsets[circle_size_offsets_idx++] = start - parse_beatmap_metadata_code_start;
    }
    ar_hook_jump_back = parse_beatmap_metadata_code_start + approach_rate_offset_2 + 0x9;
    cs_hook_jump_back = parse_beatmap_metadata_code_start + circle_size_offsets[0] + 0x9;
    ar_offsets_found = approach_rate_offset_1 && approach_rate_offset_2;
    cs_offsets_found = (bool)circle_size_offsets[2];
}

void init_hooks()
{
    if (ar_offsets_found)
    {
        ApproachRateHook1 = Hook((BYTE *)parse_beatmap_metadata_code_start + approach_rate_offset_1, (BYTE *)set_approach_rate_1, (BYTE *)&empty_trampoline, 5);
        ApproachRateHook2 = Hook((BYTE *)parse_beatmap_metadata_code_start + approach_rate_offset_2, (BYTE *)set_approach_rate_2, (BYTE *)&empty_trampoline, 9);
        if (cfg_ar_lock)
            enable_ar_hooks();
    }
    else
    {
        cfg_ar_lock = false;
    }

    if (cs_offsets_found)
    {
        CircleSizeHook_1 = Hook((BYTE *)parse_beatmap_metadata_code_start + circle_size_offsets[0], (BYTE *)set_circle_size, (BYTE *)&empty_trampoline, 9);
        CircleSizeHook_2 = Hook((BYTE *)parse_beatmap_metadata_code_start + circle_size_offsets[1], (BYTE *)set_circle_size, (BYTE *)&empty_trampoline, 9);
        CircleSizeHook_3 = Hook((BYTE *)parse_beatmap_metadata_code_start + circle_size_offsets[2], (BYTE *)set_circle_size, (BYTE *)&empty_trampoline, 9);
        if (cfg_cs_lock)
            enable_cs_hooks();
    }
    else
    {
        cfg_cs_lock = false;
    }
}

void enable_cs_hooks()
{
    CircleSizeHook_1.Enable();
    CircleSizeHook_2.Enable();
    CircleSizeHook_3.Enable();
}

void disable_cs_hooks()
{
    CircleSizeHook_1.Disable();
    CircleSizeHook_2.Disable();
    CircleSizeHook_3.Disable();
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
        jmp [ar_hook_jump_back]
    }
}

__declspec(naked) void set_approach_rate_2()
{
    __asm {
        mov eax, dword ptr [ebp-0x00000150]
        fstp dword ptr [eax+0x2C]
        mov ebx, cfg_ar_value
        mov dword ptr [eax+0x2C], ebx
        jmp [ar_hook_jump_back]
    }
}

__declspec(naked) void set_circle_size()
{
    __asm {
        mov eax, dword ptr [ebp-0x00000150]
        fstp dword ptr [eax+0x30]
        mov ebx, cfg_cs_value
        mov dword ptr [eax+0x30], ebx
        jmp [cs_hook_jump_back]
    }
}
