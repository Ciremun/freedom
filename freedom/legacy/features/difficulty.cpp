#include "legacy/features/difficulty.h"

DifficultySetting ar_setting = {
    true,                   // lock
    10.0f,                  // value
    10.0f,                  // calculated_value
    OSU_BEATMAP_AR_OFFSET,  // offset
    "AR: %.1f",             // fmt
    "AR Offsets Not Found", // error
    enable_ar_hooks,        // enable
    disable_ar_hooks,       // disable
    apply_mods_ar,          // apply_mods
    // bool found = false
};

DifficultySetting cs_setting = {
    false,                  // lock
    4.0f,                   // value
    4.0f,                   // calculated_value
    OSU_BEATMAP_CS_OFFSET,  // offset
    "CS: %.1f",             // fmt
    "CS Offsets Not Found", // error
    enable_cs_hooks,        // enable
    disable_cs_hooks,       // disable
    apply_mods_cs,          // apply_mods
    // bool found = false
};

DifficultySetting od_setting = {
    false,                  // lock
    8.0f,                   // value
    8.0f,                   // calculated_value
    OSU_BEATMAP_OD_OFFSET,  // offset
    "OD: %.1f",             // fmt
    "OD Offsets Not Found", // error
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
    if (ar_setting.found)
    {
        ApproachRateHook1 = Hook<Detour32>(approach_rate_offsets[0], (BYTE *)set_approach_rate, 9);
        ApproachRateHook2 = Hook<Detour32>(approach_rate_offsets[1], (BYTE *)set_approach_rate, 9);
        ApproachRateHook3 = Hook<Detour32>(approach_rate_offsets[2], (BYTE *)set_approach_rate_2, 12);
        if (ar_setting.enabled)
        {
            enable_ar_hooks();
            ar_setting.apply_mods();
        }
    }

    if (cs_setting.found)
    {
        CircleSizeHook1 = Hook<Detour32>(circle_size_offsets[0], (BYTE *)set_circle_size, 9);
        CircleSizeHook2 = Hook<Detour32>(circle_size_offsets[1], (BYTE *)set_circle_size, 9);
        CircleSizeHook3 = Hook<Detour32>(circle_size_offsets[2], (BYTE *)set_circle_size, 9);
        if (cs_setting.enabled)
        {
            enable_cs_hooks();
            cs_setting.apply_mods();
        }
    }

    if (od_setting.found)
    {
        OverallDifficultyHook1 = Hook<Detour32>(overall_difficulty_offsets[0], (BYTE *)set_overall_difficulty, 9);
        OverallDifficultyHook2 = Hook<Detour32>(overall_difficulty_offsets[1], (BYTE *)set_overall_difficulty, 9);
        if (od_setting.enabled)
        {
            enable_od_hooks();
            od_setting.apply_mods();
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
    od_setting.calculated_value = od_setting.value;
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
    cs_setting.calculated_value = cs_setting.value;
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
            ar_setting.calculated_value = compensate_double_time(ar_setting.value);
            FR_INFO("ar_setting.calculated_value: %.2f", ar_setting.calculated_value);
            return;
        }
    }
    ar_setting.calculated_value = ar_setting.value;
    FR_INFO("ar_setting.calculated_value: %.2f", ar_setting.calculated_value);
}

__declspec(naked) void set_approach_rate()
{
    __asm {
        mov eax, dword ptr [ebp-0x00000150]
        fstp dword ptr [eax+OSU_BEATMAP_AR_OFFSET]
        mov ebx, ar_setting.calculated_value
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
        mov ebx, ar_setting.calculated_value
        mov dword ptr [eax+OSU_BEATMAP_AR_OFFSET], ebx
        jmp [ar_hook_jump_back]
    }
}

__declspec(naked) void set_circle_size()
{
    __asm {
        mov eax, dword ptr [ebp-0x00000150]
        fstp dword ptr [eax+OSU_BEATMAP_CS_OFFSET]
        mov ebx, cs_setting.calculated_value
        mov dword ptr [eax+OSU_BEATMAP_CS_OFFSET], ebx
        jmp [cs_hook_jump_back]
    }
}

__declspec(naked) void set_overall_difficulty()
{
    __asm {
        mov eax, dword ptr [ebp-0x00000150]
        fstp dword ptr [eax+OSU_BEATMAP_OD_OFFSET]
        mov ebx, od_setting.calculated_value
        mov dword ptr [eax+OSU_BEATMAP_OD_OFFSET], ebx
        jmp [od_hook_jump_back]
    }
}
