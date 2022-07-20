#include "detours.h"

Parameter ar_parameter = {
    true,                   // lock
    10.0f,                  // value
    0x2C,                   // offset
    "AR: %.1f",             // slider_fmt
    "AR Offsets Not Found", // error_message
    enable_ar_hooks,        // enable
    disable_ar_hooks,       // disable
    // bool found = false
};

Parameter cs_parameter = {
    false,                  // lock
    4.0f,                   // value
    0x30,                   // offset
    "CS: %.1f",             // slider_fmt
    "CS Offsets Not Found", // error_message
    enable_cs_hooks,        // enable
    disable_cs_hooks,       // disable
    // bool found = false
};

Parameter od_parameter = {
    false,                  // lock
    8.0f,                   // value
    0x38,                   // offset
    "OD: %.1f",             // slider_fmt
    "OD Offsets Not Found", // error_message
    enable_od_hooks,        // enable
    disable_od_hooks,       // disable
    // bool found = false
};

std::vector<CodeStartTarget> code_starts = {
    // class, method
    {L"#=z9DEJsV779TWhgkKS1GefTZYVJDPgn5L2xrCk5pyAIyAH", L"#=zyO2ZBz4="}, // parse_beatmap
    {L"#=zZ86rRc_XTEYCVjLiIpwW9hgO85GX", L"#=zaGN2R64="},                 // beatmap_onload
    {L"#=zXYmDZ1fHfmG8nphZQw==", L"#=zuugrck8w7GV3"}                      // current scene
};

twglSwapBuffers wglSwapBuffersGateway;
void_trampoline empty_gateway;

uintptr_t parse_beatmap_metadata_code_start = 0;

uintptr_t approach_rate_offsets[2] = {0};
uintptr_t ar_hook_jump_back = 0;

uintptr_t circle_size_offsets[3] = {0};
uintptr_t cs_hook_jump_back = 0;

uintptr_t overall_difficulty_offsets[2] = {0};
uintptr_t od_hook_jump_back = 0;

uintptr_t beatmap_onload_code_start = 0;
uintptr_t beatmap_onload_offset = 0;
uintptr_t beatmap_onload_hook_jump_back = 0;

uintptr_t current_scene_code_start = 0;
uintptr_t current_scene_offset = 0;
uintptr_t notify_on_scene_change_original_mov_address = 0;
uintptr_t current_scene_hook_jump_back = 0;

Hook SwapBuffersHook;

Hook ApproachRateHook1;
Hook ApproachRateHook2;

Hook CircleSizeHook_1;
Hook CircleSizeHook_2;
Hook CircleSizeHook_3;

Hook OverallDifficultyHook_1;
Hook OverallDifficultyHook_2;

Hook BeatmapOnLoadHook;
Hook SceneChangeHook;

void try_find_hook_offsets()
{
    code_start_for_class_methods(code_starts);
    parse_beatmap_metadata_code_start = code_starts[0].start;
    beatmap_onload_code_start = code_starts[1].start;
    current_scene_code_start = code_starts[2].start;
    FR_INFO_FMT("parse_beatmap_metadata_code_start: 0x%X", parse_beatmap_metadata_code_start);
    FR_INFO_FMT("beatmap_onload_code_start: 0x%X", beatmap_onload_code_start);
    FR_INFO_FMT("current_scene_code_start: 0x%X", current_scene_code_start);
    if (parse_beatmap_metadata_code_start)
    {
        const uint8_t approach_rate_signature[] = {0x8B, 0x85, 0xB0, 0xFE, 0xFF, 0xFF, 0xD9, 0x58, 0x2C};
        const uint8_t circle_size_signature[] = {0x8B, 0x85, 0xB0, 0xFE, 0xFF, 0xFF, 0xD9, 0x58, 0x30};
        const uint8_t overall_difficulty_signature[] = {0x8B, 0x85, 0xB0, 0xFE, 0xFF, 0xFF, 0xD9, 0x58, 0x38};
        int approach_rate_offsets_idx = 0;
        int circle_size_offsets_idx = 0;
        int overall_difficulty_offsets_idx = 0;
        for (uintptr_t start = parse_beatmap_metadata_code_start + 0x1000; start - parse_beatmap_metadata_code_start <= 0x1CFF; ++start)
        {
            if (approach_rate_offsets_idx < 2 &&
                memcmp((uint8_t *)start, approach_rate_signature, sizeof(approach_rate_signature)) == 0)
                approach_rate_offsets[approach_rate_offsets_idx++] = start - parse_beatmap_metadata_code_start;
            if (circle_size_offsets_idx < 3 &&
                memcmp((uint8_t *)start, circle_size_signature, sizeof(circle_size_signature)) == 0)
                circle_size_offsets[circle_size_offsets_idx++] = start - parse_beatmap_metadata_code_start;
            if (overall_difficulty_offsets_idx < 2 &&
                memcmp((uint8_t *)start, overall_difficulty_signature, sizeof(overall_difficulty_signature)) == 0)
                overall_difficulty_offsets[overall_difficulty_offsets_idx++] = start - parse_beatmap_metadata_code_start;
        }
        ar_hook_jump_back = parse_beatmap_metadata_code_start + approach_rate_offsets[1] + 0x9;
        cs_hook_jump_back = parse_beatmap_metadata_code_start + circle_size_offsets[0] + 0x9;
        od_hook_jump_back = parse_beatmap_metadata_code_start + overall_difficulty_offsets[1] + 0x9;
        ar_parameter.found = approach_rate_offsets[1] > 0;
        cs_parameter.found = circle_size_offsets[2] > 0;
        od_parameter.found = overall_difficulty_offsets[1] > 0;
    }
    if (beatmap_onload_code_start)
    {
        const uint8_t beatmap_onload_signature[] = {0x8B, 0x86, 0x48, 0x01, 0x00, 0x00};
        for (uintptr_t start = beatmap_onload_code_start + 0x100; start - beatmap_onload_code_start <= 0x300; ++start)
        {
            if (memcmp((uint8_t *)start, beatmap_onload_signature, sizeof(beatmap_onload_signature)) == 0)
            {
                beatmap_onload_offset = start - beatmap_onload_code_start;
                beatmap_onload_hook_jump_back = beatmap_onload_code_start + beatmap_onload_offset + 0x6;
                break;
            }
        }
    }
    FR_INFO_FMT("beatmap_onload_offset: 0x%X", beatmap_onload_offset);
    if (current_scene_code_start)
    {
        const uint8_t current_scene_signature[] = {0xA1, 0xA3, 0xA1, 0xA3};
        for (uintptr_t start = current_scene_code_start + 0x18; start - current_scene_code_start <= 0x800; ++start)
        {
            uint8_t *bytes = (uint8_t *)start;
            const uint8_t signature[] = {bytes[0], bytes[5], bytes[10], bytes[15]};
            if (memcmp(signature, current_scene_signature, sizeof(current_scene_signature)) == 0)
            {
                current_scene_offset = start - current_scene_code_start + 0xF;
                current_scene_hook_jump_back = current_scene_code_start + current_scene_offset + 0x5;
                notify_on_scene_change_original_mov_address = *(uintptr_t *)(current_scene_code_start + current_scene_offset + 0x1);
                break;
            }
        }
    }
    FR_INFO_FMT("current_scene_offset: 0x%X", current_scene_offset);
}

void init_hooks()
{
    if (ar_parameter.found)
    {
        ApproachRateHook1 = Hook((BYTE *)parse_beatmap_metadata_code_start + approach_rate_offsets[0], (BYTE *)set_approach_rate, (BYTE *)&empty_gateway, 9);
        ApproachRateHook2 = Hook((BYTE *)parse_beatmap_metadata_code_start + approach_rate_offsets[1], (BYTE *)set_approach_rate, (BYTE *)&empty_gateway, 9);
        if (ar_parameter.lock)
            enable_ar_hooks();
    }
    else
        ar_parameter.lock = false;

    if (cs_parameter.found)
    {
        CircleSizeHook_1 = Hook((BYTE *)parse_beatmap_metadata_code_start + circle_size_offsets[0], (BYTE *)set_circle_size, (BYTE *)&empty_gateway, 9);
        CircleSizeHook_2 = Hook((BYTE *)parse_beatmap_metadata_code_start + circle_size_offsets[1], (BYTE *)set_circle_size, (BYTE *)&empty_gateway, 9);
        CircleSizeHook_3 = Hook((BYTE *)parse_beatmap_metadata_code_start + circle_size_offsets[2], (BYTE *)set_circle_size, (BYTE *)&empty_gateway, 9);
        if (cs_parameter.lock)
            enable_cs_hooks();
    }
    else
        cs_parameter.lock = false;

    if (od_parameter.found)
    {
        OverallDifficultyHook_1 = Hook((BYTE *)parse_beatmap_metadata_code_start + overall_difficulty_offsets[0], (BYTE *)set_overall_difficulty, (BYTE *)&empty_gateway, 9);
        OverallDifficultyHook_2 = Hook((BYTE *)parse_beatmap_metadata_code_start + overall_difficulty_offsets[1], (BYTE *)set_overall_difficulty, (BYTE *)&empty_gateway, 9);
        if (od_parameter.lock)
            enable_od_hooks();
    }
    else
        od_parameter.lock = false;

    if (beatmap_onload_offset)
    {
        BeatmapOnLoadHook = Hook((BYTE *)beatmap_onload_code_start + beatmap_onload_offset, (BYTE *)notify_on_beatmap_load, (BYTE *)&empty_gateway, 6);
        BeatmapOnLoadHook.Enable();
    }

    if (current_scene_offset)
    {
        SceneChangeHook = Hook((BYTE *)current_scene_code_start + current_scene_offset, (BYTE *)notify_on_scene_change, (BYTE *)&empty_gateway, 5);
        SceneChangeHook.Enable();
    }
}

void enable_od_hooks()
{
    OverallDifficultyHook_1.Enable();
    OverallDifficultyHook_2.Enable();
}

void disable_od_hooks()
{
    OverallDifficultyHook_1.Disable();
    OverallDifficultyHook_2.Disable();
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

__declspec(naked) void notify_on_beatmap_load()
{
    __asm {
        mov beatmap_loaded, 1
        mov hit_objects_ms_idx, 0
        mov eax, [esi+0x00000148]
        jmp [beatmap_onload_hook_jump_back]
    }
}

static void set_beatmap_loaded_on_scene_change()
{
    if (current_scene != Scene::GAMIN)
    {
        beatmap_loaded = false;
        hit_objects_ms_idx = 0;
    }
}

__declspec(naked) void notify_on_scene_change()
{
    __asm {
        mov current_scene, eax
        mov edx, notify_on_scene_change_original_mov_address
        mov dword ptr [edx], eax
        mov edx, 0
        call set_beatmap_loaded_on_scene_change
        jmp [current_scene_hook_jump_back]
    }
}
