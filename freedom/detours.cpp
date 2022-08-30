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

bool cfg_relax_lock = false;
bool cfg_aimbot_lock = false;

std::vector<CodeStartTarget> code_starts = {
    // class, method
    {L"#=zk8tPJ343CtdfCoyZXqxpXL8Mpr0BtBM7puFf$KCLvSvu", L"#=zmR99pQ0="}, // parse_beatmap - fixed
    {L"#=zlfVZLHpJLMIvTtJK$8MwYPK4xDI4", L"#=zhlImBbU="},                 // beatmap_onload - fixed
    {L"#=zmGcBavNsjoYtDFRNGA==", L"#=zbSsRq67idAjk"},                     // current scene - fixed
    {L"#=z60q2XH9640DxZLQ$1A==", L"#=zsYVujtWLSR28"},                     // selected song, audio time - fixed
    {L"#=znb3Y$KPwyjSF$10aUA==", L"#=zgiDWpY4uZHdja87yp7x7W1U="},         // osu manager - fixed
    {L"#=zXCB2DjGO4mRaBoN27A==", L"#=zUeWVlwc="},                         // binding manager
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

uintptr_t selected_song_code_start = 0;
uintptr_t selected_song_ptr = 0;

uintptr_t audio_time_code_start = 0;
uintptr_t audio_time_ptr = 0;

uintptr_t osu_manager_code_start = 0;
uintptr_t osu_manager_ptr = 0;

uintptr_t binding_manager_code_start = 0;
uintptr_t binding_manager_ptr = 0;

Hook SwapBuffersHook;

Hook ApproachRateHook1;
Hook ApproachRateHook2;

Hook CircleSizeHook1;
Hook CircleSizeHook2;
Hook CircleSizeHook3;

Hook OverallDifficultyHook1;
Hook OverallDifficultyHook2;

Hook BeatmapOnLoadHook;
Hook SceneChangeHook;

void try_find_hook_offsets()
{
    code_start_for_class_methods(code_starts);
    parse_beatmap_metadata_code_start = code_starts[0].start;
    beatmap_onload_code_start = code_starts[1].start;
    current_scene_code_start = code_starts[2].start;
    selected_song_code_start = code_starts[3].start;
    audio_time_code_start = code_starts[3].start;
    osu_manager_code_start = code_starts[4].start;
    binding_manager_code_start = code_starts[5].start;
    FR_PTR_INFO("parse_beatmap_metadata_code_start", parse_beatmap_metadata_code_start);
    if (parse_beatmap_metadata_code_start)
    {
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
        FR_INFO_FMT("ar_parameter.found: %d", ar_parameter.found);
        FR_INFO_FMT("cs_parameter.found: %d", cs_parameter.found);
        FR_INFO_FMT("od_parameter.found: %d", od_parameter.found);
    }
    FR_PTR_INFO("current_scene_code_start", current_scene_code_start);
    if (current_scene_code_start)
    {
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
        FR_PTR_INFO("current_scene_offset", current_scene_offset);
    }
    FR_PTR_INFO("beatmap_onload_code_start", beatmap_onload_code_start);
    if (beatmap_onload_code_start)
    {
        beatmap_onload_offset = find_opcodes(beatmap_onload_signature, beatmap_onload_code_start, 0x100, 0x300);
        beatmap_onload_hook_jump_back = beatmap_onload_code_start + beatmap_onload_offset + 0x6;
        FR_PTR_INFO("beatmap_onload_offset", beatmap_onload_offset);
    }
    FR_PTR_INFO("selected_song_code_start", selected_song_code_start);
    if (selected_song_code_start)
    {
        uintptr_t selected_song_offset = find_opcodes(selected_song_signature, selected_song_code_start, 0x200, 0x5A6);
        selected_song_ptr = *(uintptr_t *)(selected_song_code_start + selected_song_offset + 0x8);
        FR_PTR_INFO("selected_song_ptr", selected_song_ptr);
    }
    FR_PTR_INFO("audio_time_code_start", audio_time_code_start);
    if (audio_time_code_start)
    {
        uintptr_t audio_time_offset = find_opcodes(audio_time_signature, audio_time_code_start, 0x0, 0x5A6);
        audio_time_ptr = *(uintptr_t *)(audio_time_code_start + audio_time_offset - 0xA);
        FR_PTR_INFO("audio_time_ptr", audio_time_ptr);
    }
    FR_PTR_INFO("osu_manager_code_start", osu_manager_code_start);
    if (osu_manager_code_start)
    {
        uintptr_t osu_manager_offset = find_opcodes(osu_manager_signature, osu_manager_code_start, 0x0, 0x100);
        osu_manager_ptr = *(uintptr_t *)(osu_manager_code_start + osu_manager_offset + 0x3);
        FR_PTR_INFO("osu_manager_ptr", osu_manager_ptr);
    }
    FR_PTR_INFO("binding_manager_code_start", binding_manager_code_start);
    if (binding_manager_code_start)
    {
        uintptr_t binding_manager_offset = find_opcodes(binding_manager_signature, binding_manager_code_start, 0x0, 0x100);
        uintptr_t unknown_1 = **(uintptr_t **)(binding_manager_code_start + binding_manager_offset + 0x6);
        uintptr_t unknown_2 = *(uintptr_t *)(unknown_1 + 0x8);
        binding_manager_ptr = unknown_2 + 0x14;
        FR_PTR_INFO("binding_manager_ptr", binding_manager_ptr);
    }
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
        CircleSizeHook1 = Hook((BYTE *)parse_beatmap_metadata_code_start + circle_size_offsets[0], (BYTE *)set_circle_size, (BYTE *)&empty_gateway, 9);
        CircleSizeHook2 = Hook((BYTE *)parse_beatmap_metadata_code_start + circle_size_offsets[1], (BYTE *)set_circle_size, (BYTE *)&empty_gateway, 9);
        CircleSizeHook3 = Hook((BYTE *)parse_beatmap_metadata_code_start + circle_size_offsets[2], (BYTE *)set_circle_size, (BYTE *)&empty_gateway, 9);
        if (cs_parameter.lock)
            enable_cs_hooks();
    }
    else
        cs_parameter.lock = false;

    if (od_parameter.found)
    {
        OverallDifficultyHook1 = Hook((BYTE *)parse_beatmap_metadata_code_start + overall_difficulty_offsets[0], (BYTE *)set_overall_difficulty, (BYTE *)&empty_gateway, 9);
        OverallDifficultyHook2 = Hook((BYTE *)parse_beatmap_metadata_code_start + overall_difficulty_offsets[1], (BYTE *)set_overall_difficulty, (BYTE *)&empty_gateway, 9);
        if (od_parameter.lock)
            enable_od_hooks();
    }
    else
        od_parameter.lock = false;

    if (beatmap_onload_offset)
    {
        BeatmapOnLoadHook = Hook((BYTE *)beatmap_onload_code_start + beatmap_onload_offset, (BYTE *)notify_on_beatmap_load, (BYTE *)&empty_gateway, 6);
        if (cfg_relax_lock || cfg_aimbot_lock)
            BeatmapOnLoadHook.Enable();
    }

    if (current_scene_offset)
    {
        SceneChangeHook = Hook((BYTE *)current_scene_code_start + current_scene_offset, (BYTE *)notify_on_scene_change, (BYTE *)&empty_gateway, 5);
        if (cfg_relax_lock || cfg_aimbot_lock)
            SceneChangeHook.Enable();
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
}

void disable_ar_hooks()
{
    ApproachRateHook1.Disable();
    ApproachRateHook2.Disable();
}

void enable_notify_hooks()
{
    if (!cfg_relax_lock || !cfg_aimbot_lock)
    {
        BeatmapOnLoadHook.Enable();
        SceneChangeHook.Enable();
    }
}

void disable_notify_hooks()
{
    if (!cfg_relax_lock && !cfg_aimbot_lock)
    {
        BeatmapOnLoadHook.Disable();
        SceneChangeHook.Disable();
    }
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
        mov start_parse_beatmap, 1
        mov eax, [esi+0x00000148]
        jmp [beatmap_onload_hook_jump_back]
    }
}

__declspec(naked) void notify_on_scene_change()
{
    __asm {
        mov current_scene, eax
        mov edx, notify_on_scene_change_original_mov_address
        mov dword ptr [edx], eax
        mov edx, 0
        jmp [current_scene_hook_jump_back]
    }
}
