#include "features/difficulty.h"

Parameter ar_parameter = {
    true,                   // lock
    10.0f,                  // value
    OSU_BEATMAP_AR_OFFSET,  // offset
    "AR: %.1f",             // slider_fmt
    "AR Offsets Not Found", // error_message
    enable_ar_hooks,        // enable
    disable_ar_hooks,       // disable
    // bool found = false
};

Parameter cs_parameter = {
    false,                  // lock
    4.0f,                   // value
    OSU_BEATMAP_CS_OFFSET,  // offset
    "CS: %.1f",             // slider_fmt
    "CS Offsets Not Found", // error_message
    enable_cs_hooks,        // enable
    disable_cs_hooks,       // disable
    // bool found = false
};

Parameter od_parameter = {
    false,                  // lock
    8.0f,                   // value
    OSU_BEATMAP_OD_OFFSET,  // offset
    "OD: %.1f",             // slider_fmt
    "OD Offsets Not Found", // error_message
    enable_od_hooks,        // enable
    disable_od_hooks,       // disable
    // bool found = false
};

uintptr_t parse_beatmap_code_start = 0;

uintptr_t approach_rate_offsets[3] = {0};
uintptr_t ar_hook_jump_back = 0;

uintptr_t circle_size_offsets[3] = {0};
uintptr_t cs_hook_jump_back = 0;

uintptr_t overall_difficulty_offsets[2] = {0};
uintptr_t od_hook_jump_back = 0;

Hook<Detour32> ApproachRateHook1;
Hook<Detour32> ApproachRateHook2;
Hook<Detour32> ApproachRateHook3;

Hook<Detour32> CircleSizeHook1;
Hook<Detour32> CircleSizeHook2;
Hook<Detour32> CircleSizeHook3;

Hook<Detour32> OverallDifficultyHook1;
Hook<Detour32> OverallDifficultyHook2;

void init_difficulty()
{
    if (ar_parameter.found)
    {
        ApproachRateHook1 = Hook<Detour32>(approach_rate_offsets[0], (BYTE *)set_approach_rate, 9);
        ApproachRateHook2 = Hook<Detour32>(approach_rate_offsets[1], (BYTE *)set_approach_rate, 9);
        ApproachRateHook3 = Hook<Detour32>(approach_rate_offsets[2], (BYTE *)set_approach_rate_2, 12);
        if (ar_parameter.lock)
            enable_ar_hooks();
    }

    if (cs_parameter.found)
    {
        CircleSizeHook1 = Hook<Detour32>(circle_size_offsets[0], (BYTE *)set_circle_size, 9);
        CircleSizeHook2 = Hook<Detour32>(circle_size_offsets[1], (BYTE *)set_circle_size, 9);
        CircleSizeHook3 = Hook<Detour32>(circle_size_offsets[2], (BYTE *)set_circle_size, 9);
        if (cs_parameter.lock)
            enable_cs_hooks();
    }

    if (od_parameter.found)
    {
        OverallDifficultyHook1 = Hook<Detour32>(overall_difficulty_offsets[0], (BYTE *)set_overall_difficulty, 9);
        OverallDifficultyHook2 = Hook<Detour32>(overall_difficulty_offsets[1], (BYTE *)set_overall_difficulty, 9);
        if (od_parameter.lock)
            enable_od_hooks();
    }
}

void enable_od_hooks()
{
    OverallDifficultyHook1.Enable();
    OverallDifficultyHook2.Enable();
}

void disable_od_hooks()
{
    OverallDifficultyHook1.Disable();
    OverallDifficultyHook2.Disable();
}

void enable_cs_hooks()
{
    CircleSizeHook1.Enable();
    CircleSizeHook2.Enable();
    CircleSizeHook3.Enable();
}

void disable_cs_hooks()
{
    CircleSizeHook1.Disable();
    CircleSizeHook2.Disable();
    CircleSizeHook3.Disable();
}

void enable_ar_hooks()
{
    ApproachRateHook1.Enable();
    ApproachRateHook2.Enable();
    ApproachRateHook3.Enable();
}

void disable_ar_hooks()
{
    ApproachRateHook1.Disable();
    ApproachRateHook2.Disable();
    ApproachRateHook3.Disable();
}

__declspec(naked) void set_approach_rate()
{
    __asm {
        mov eax, dword ptr [ebp-0x00000150]
        fstp dword ptr [eax+0x2C]
        mov ebx, ar_parameter.value
        mov dword ptr [eax+0x2C], ebx
        jmp [ar_hook_jump_back]
    }
}

__declspec(naked) void set_approach_rate_2()
{
    __asm {
        mov eax,[ebp-0x00000150]
        fld dword ptr [eax+0x38]
        fstp dword ptr [eax+0x2C]
        mov ebx, ar_parameter.value
        mov dword ptr [eax+0x2C], ebx
        jmp [ar_hook_jump_back]
    }
}

__declspec(naked) void set_circle_size()
{
    __asm {
        mov eax, dword ptr [ebp-0x00000150]
        fstp dword ptr [eax+0x30]
        mov ebx, cs_parameter.value
        mov dword ptr [eax+0x30], ebx
        jmp [cs_hook_jump_back]
    }
}

__declspec(naked) void set_overall_difficulty()
{
    __asm {
        mov eax, dword ptr [ebp-0x00000150]
        fstp dword ptr [eax+0x38]
        mov ebx, od_parameter.value
        mov dword ptr [eax+0x38], ebx
        jmp [od_hook_jump_back]
    }
}
