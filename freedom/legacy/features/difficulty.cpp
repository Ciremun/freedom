#include "features/difficulty.h"

Parameter ar_parameter = {
    true,                   // lock
    10.0f,                  // value
    10.0f,                  // calculated_value
    OSU_BEATMAP_AR_OFFSET,  // offset
    "AR: %.1f",             // slider_fmt
    "AR Offsets Not Found", // error_message
    enable_ar_hooks,        // enable
    disable_ar_hooks,       // disable
    apply_mods_ar,          // apply_mods
    // bool found = false
};

Parameter cs_parameter = {
    false,                  // lock
    4.0f,                   // value
    4.0f,                   // calculated_value
    OSU_BEATMAP_CS_OFFSET,  // offset
    "CS: %.1f",             // slider_fmt
    "CS Offsets Not Found", // error_message
    enable_cs_hooks,        // enable
    disable_cs_hooks,       // disable
    apply_mods_cs,          // apply_mods
    // bool found = false
};

Parameter od_parameter = {
    false,                  // lock
    8.0f,                   // value
    8.0f,                   // calculated_value
    OSU_BEATMAP_OD_OFFSET,  // offset
    "OD: %.1f",             // slider_fmt
    "OD Offsets Not Found", // error_message
    enable_od_hooks,        // enable
    disable_od_hooks,       // disable
    apply_mods_od,          // apply_mods
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

static inline float ms_to_ar(float ms)
{
    return ms > 1200.f ? (1800.f - ms) / 120.f : (1200.f - ms) / 150.f + 5.f;
}

static inline float ar_to_ms(float ar)
{
    return ar <= 5.f ? 1200.f + 600.f * (5.f - ar) / 5.f : 1200.f - 750.f * (ar - 5.f) / 5.f;
}

static inline float compensate_double_time(float ar)
{
    return max(ms_to_ar(ar_to_ms(ar) * 1.5f), .0f);
}

void init_difficulty()
{
    if (ar_parameter.found)
    {
        ApproachRateHook1 = Hook<Detour32>(approach_rate_offsets[0], (BYTE *)set_approach_rate, 9);
        ApproachRateHook2 = Hook<Detour32>(approach_rate_offsets[1], (BYTE *)set_approach_rate, 9);
        ApproachRateHook3 = Hook<Detour32>(approach_rate_offsets[2], (BYTE *)set_approach_rate_2, 12);
        if (ar_parameter.lock)
        {
            enable_ar_hooks();
            ar_parameter.apply_mods();
        }
    }

    if (cs_parameter.found)
    {
        CircleSizeHook1 = Hook<Detour32>(circle_size_offsets[0], (BYTE *)set_circle_size, 9);
        CircleSizeHook2 = Hook<Detour32>(circle_size_offsets[1], (BYTE *)set_circle_size, 9);
        CircleSizeHook3 = Hook<Detour32>(circle_size_offsets[2], (BYTE *)set_circle_size, 9);
        if (cs_parameter.lock)
        {
            enable_cs_hooks();
            cs_parameter.apply_mods();
        }
    }

    if (od_parameter.found)
    {
        OverallDifficultyHook1 = Hook<Detour32>(overall_difficulty_offsets[0], (BYTE *)set_overall_difficulty, 9);
        OverallDifficultyHook2 = Hook<Detour32>(overall_difficulty_offsets[1], (BYTE *)set_overall_difficulty, 9);
        if (od_parameter.lock)
        {
            enable_od_hooks();
            od_parameter.apply_mods();
        }
    }
}

void enable_od_hooks()
{
    enable_update_mods_hook();
    OverallDifficultyHook1.Enable();
    OverallDifficultyHook2.Enable();
}

void disable_od_hooks()
{
    disable_update_mods_hook();
    OverallDifficultyHook1.Disable();
    OverallDifficultyHook2.Disable();
}

void apply_mods_od()
{
    od_parameter.calculated_value = od_parameter.value;
}

void enable_cs_hooks()
{
    enable_update_mods_hook();
    CircleSizeHook1.Enable();
    CircleSizeHook2.Enable();
    CircleSizeHook3.Enable();
}

void disable_cs_hooks()
{
    disable_update_mods_hook();
    CircleSizeHook1.Disable();
    CircleSizeHook2.Disable();
    CircleSizeHook3.Disable();
}

void apply_mods_cs()
{
    cs_parameter.calculated_value = cs_parameter.value;
}

void enable_ar_hooks()
{
    enable_update_mods_hook();
    ApproachRateHook1.Enable();
    ApproachRateHook2.Enable();
    ApproachRateHook3.Enable();
}

void disable_ar_hooks()
{
    disable_update_mods_hook();
    ApproachRateHook1.Disable();
    ApproachRateHook2.Disable();
    ApproachRateHook3.Disable();
}

void apply_mods_ar()
{
    if (selected_mods_ptr)
    {
        Mods mods = *selected_mods_ptr;
        if ((mods & Mods::Nightcore) || (mods & Mods::DoubleTime))
        {
            ar_parameter.calculated_value = compensate_double_time(ar_parameter.value);
            FR_INFO("ar_parameter.calculated_value: %.2f", ar_parameter.calculated_value);
            return;
        }
    }
    ar_parameter.calculated_value = ar_parameter.value;
    FR_INFO("ar_parameter.calculated_value: %.2f", ar_parameter.calculated_value);
}

__declspec(naked) void set_approach_rate()
{
    __asm {
        mov eax, dword ptr [ebp-0x00000150]
        fstp dword ptr [eax+OSU_BEATMAP_AR_OFFSET]
        mov ebx, ar_parameter.calculated_value
        mov dword ptr [eax+OSU_BEATMAP_AR_OFFSET], ebx
        jmp [ar_hook_jump_back]
    }
}

__declspec(naked) void set_approach_rate_2()
{
    __asm {
        mov eax,[ebp-0x00000150]
        fld dword ptr [eax+0x38]
        fstp dword ptr [eax+OSU_BEATMAP_AR_OFFSET]
        mov ebx, ar_parameter.calculated_value
        mov dword ptr [eax+OSU_BEATMAP_AR_OFFSET], ebx
        jmp [ar_hook_jump_back]
    }
}

__declspec(naked) void set_circle_size()
{
    __asm {
        mov eax, dword ptr [ebp-0x00000150]
        fstp dword ptr [eax+OSU_BEATMAP_CS_OFFSET]
        mov ebx, cs_parameter.calculated_value
        mov dword ptr [eax+OSU_BEATMAP_CS_OFFSET], ebx
        jmp [cs_hook_jump_back]
    }
}

__declspec(naked) void set_overall_difficulty()
{
    __asm {
        mov eax, dword ptr [ebp-0x00000150]
        fstp dword ptr [eax+OSU_BEATMAP_OD_OFFSET]
        mov ebx, od_parameter.calculated_value
        mov dword ptr [eax+OSU_BEATMAP_OD_OFFSET], ebx
        jmp [od_hook_jump_back]
    }
}
