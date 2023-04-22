// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

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

twglSwapBuffers wglSwapBuffersGateway;

uintptr_t parse_beatmap_code_start = 0;

uintptr_t approach_rate_offsets[3] = {0};
uintptr_t ar_hook_jump_back = 0;

uintptr_t circle_size_offsets[3] = {0};
uintptr_t cs_hook_jump_back = 0;

uintptr_t overall_difficulty_offsets[2] = {0};
uintptr_t od_hook_jump_back = 0;

uintptr_t beatmap_onload_code_start = 0;
uintptr_t beatmap_onload_offset = 0;
uintptr_t beatmap_onload_hook_jump_back = 0;

uintptr_t score_multiplier_code_start = 0;
uintptr_t score_multiplier_hook_jump_back = 0;

uintptr_t current_scene_code_start = 0;
uintptr_t current_scene_offset = 0;
Scene *current_scene_ptr = 0;

uintptr_t selected_song_code_start = 0;
uintptr_t selected_song_ptr = 0;

uintptr_t audio_time_code_start = 0;
uintptr_t audio_time_ptr = 0;

uintptr_t osu_manager_code_start = 0;
uintptr_t osu_manager_ptr = 0;

uintptr_t binding_manager_code_start = 0;
uintptr_t binding_manager_ptr = 0;

uintptr_t selected_replay_code_start = 0;
uintptr_t selected_replay_offset = 0;
uintptr_t selected_replay_hook_jump_back = 0;
uintptr_t selected_replay_ptr = 0;

uintptr_t window_manager_code_start = 0;
uintptr_t window_manager_offset = 0;
uintptr_t window_manager_ptr = 0;

uintptr_t nt_user_send_input_ptr = 0;
uintptr_t nt_user_send_input_original_jmp_address = 0;
uintptr_t dispatch_table_id = 0x0000107F;
uintptr_t nt_user_send_input_dispatch_table_id_found = false;

uintptr_t osu_client_id_code_start = 0;
char osu_client_id[64] = {0};

uintptr_t osu_username_code_start = 0;
char osu_username[32] = {0};

Hook<Trampoline32> SwapBuffersHook;

Hook<Detour32> ApproachRateHook1;
Hook<Detour32> ApproachRateHook2;
Hook<Detour32> ApproachRateHook3;

Hook<Detour32> CircleSizeHook1;
Hook<Detour32> CircleSizeHook2;
Hook<Detour32> CircleSizeHook3;

Hook<Detour32> OverallDifficultyHook1;
Hook<Detour32> OverallDifficultyHook2;

Hook<Detour32> BeatmapOnLoadHook;

Hook<Detour32> SelectedReplayHook;

Hook<Detour32> ScoreMultiplierHook;

float memory_scan_progress = .0f;

static inline bool all_code_starts_found()
{
    return parse_beatmap_code_start && beatmap_onload_code_start && current_scene_code_start && selected_song_code_start &&
        audio_time_code_start && osu_manager_code_start && binding_manager_code_start && selected_replay_code_start &&
        osu_client_id_code_start && osu_username_code_start && window_manager_code_start && nt_user_send_input_dispatch_table_id_found && score_multiplier_code_start;
}

int filter(unsigned int code, struct _EXCEPTION_POINTERS *ep)
{
    if (code == EXCEPTION_ACCESS_VIOLATION)
    {
        return EXCEPTION_EXECUTE_HANDLER;
    }
    else
    {
        return EXCEPTION_CONTINUE_SEARCH;
    };
}

static inline bool is_dispatch_table_id(uint8_t *opcodes)
{
    return *opcodes == (uint8_t)0xB8 && opcodes[5] == (uint8_t)0xE9 &&
          (*(uintptr_t*)(opcodes + 0x6) + (uintptr_t)opcodes + 0x5 + 0x5) == (uintptr_t)(nt_user_send_input_ptr + 0x5);
}

static void scan_for_code_starts()
{
    prejit_all();
    const auto find_code_start = [](uint8_t *opcodes, uintptr_t &code_start, uint8_t *code_start_signature, size_t code_start_signature_size)
    {
        if (!code_start && memcmp(opcodes, code_start_signature, code_start_signature_size) == 0)
            code_start = (uintptr_t)opcodes;
    };

    scan_memory(GetModuleBaseAddress(L"osu!.exe"), 0x7FFFFFFF, 8, [&](uintptr_t begin, int alignment, unsigned char *block, unsigned int idx)
    {
        __try
        {
            uint8_t *opcodes = (uint8_t *)(begin + idx * alignment);
            memory_scan_progress = (uintptr_t)opcodes / (float)0x7FFFFFFF;
            find_code_start(opcodes, parse_beatmap_code_start,    (uint8_t *)parse_beatmap_function_signature,   sizeof(parse_beatmap_function_signature));
            find_code_start(opcodes, beatmap_onload_code_start,   (uint8_t *)beatmap_onload_function_signature,  sizeof(beatmap_onload_function_signature));
            find_code_start(opcodes, current_scene_code_start,    (uint8_t *)current_scene_function_signature,   sizeof(current_scene_function_signature));
            find_code_start(opcodes, selected_song_code_start,    (uint8_t *)selected_song_function_signature,   sizeof(selected_song_function_signature));
            find_code_start(opcodes, audio_time_code_start,       (uint8_t *)audio_time_function_signature,      sizeof(audio_time_function_signature));
            find_code_start(opcodes, osu_manager_code_start,      (uint8_t *)osu_manager_function_signature,     sizeof(osu_manager_function_signature));
            find_code_start(opcodes, binding_manager_code_start,  (uint8_t *)binding_manager_function_signature, sizeof(binding_manager_function_signature));
            find_code_start(opcodes, selected_replay_code_start,  (uint8_t *)selected_replay_function_signature, sizeof(selected_replay_function_signature));
            find_code_start(opcodes, osu_client_id_code_start,    (uint8_t *)osu_client_id_function_signature,   sizeof(osu_client_id_function_signature));
            find_code_start(opcodes, osu_username_code_start,     (uint8_t *)username_function_signature,        sizeof(username_function_signature));
            find_code_start(opcodes, window_manager_code_start,   (uint8_t *)window_manager_function_signature,  sizeof(window_manager_function_signature));
            find_code_start(opcodes, score_multiplier_code_start, (uint8_t *)score_multiplier_signature,         sizeof(score_multiplier_signature));

            if (!nt_user_send_input_dispatch_table_id_found && is_dispatch_table_id(opcodes))
            {
                dispatch_table_id = *(uintptr_t *)(opcodes + 0x1);
                nt_user_send_input_dispatch_table_id_found = true;
                FR_INFO_FMT("found dispatch_table_id: %X", dispatch_table_id);
            }
        }
        __except(filter(GetExceptionCode(), GetExceptionInformation()))
        {
            FR_PTR_INFO("exception in scan_for_code_starts", begin + idx * alignment);
        }

        return all_code_starts_found();
    });

    memory_scan_progress = 1.f;
}

static void try_find_hook_offsets()
{
    FR_PTR_INFO("parse_beatmap_code_start", parse_beatmap_code_start);
    if (parse_beatmap_code_start)
    {
        int approach_rate_offsets_idx = 0;
        int circle_size_offsets_idx = 0;
        int overall_difficulty_offsets_idx = 0;
        for (uintptr_t start = parse_beatmap_code_start + 0x1000; start - parse_beatmap_code_start <= 0x1CFF; ++start)
        {
            if (approach_rate_offsets_idx < 2 &&
                memcmp((uint8_t *)start, approach_rate_signature, sizeof(approach_rate_signature)) == 0)
                approach_rate_offsets[approach_rate_offsets_idx++] = start - parse_beatmap_code_start;
            if (!approach_rate_offsets[2] &&
                memcmp((uint8_t *)start, approach_rate_signature_2, sizeof(approach_rate_signature_2)) == 0)
                approach_rate_offsets[2] = start - parse_beatmap_code_start;
            if (circle_size_offsets_idx < 3 &&
                memcmp((uint8_t *)start, circle_size_signature, sizeof(circle_size_signature)) == 0)
                circle_size_offsets[circle_size_offsets_idx++] = start - parse_beatmap_code_start;
            if (overall_difficulty_offsets_idx < 2 &&
                memcmp((uint8_t *)start, overall_difficulty_signature, sizeof(overall_difficulty_signature)) == 0)
                overall_difficulty_offsets[overall_difficulty_offsets_idx++] = start - parse_beatmap_code_start;
        }
        ar_hook_jump_back = parse_beatmap_code_start + approach_rate_offsets[1] + 0x9;
        cs_hook_jump_back = parse_beatmap_code_start + circle_size_offsets[0] + 0x9;
        od_hook_jump_back = parse_beatmap_code_start + overall_difficulty_offsets[1] + 0x9;
        ar_parameter.found = approach_rate_offsets[1] > 0 && approach_rate_offsets[2] > 0;
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
                current_scene_ptr = *(Scene **)(current_scene_code_start + current_scene_offset + 0x1);
                break;
            }
        }
        FR_PTR_INFO("current_scene_offset", current_scene_offset);
    }
    FR_PTR_INFO("beatmap_onload_code_start", beatmap_onload_code_start);
    if (beatmap_onload_code_start)
    {
        beatmap_onload_offset = find_opcodes(beatmap_onload_signature, beatmap_onload_code_start, 0x50, 0x300);
        if (beatmap_onload_offset)
            beatmap_onload_hook_jump_back = beatmap_onload_code_start + beatmap_onload_offset + 0x6;
        FR_PTR_INFO("beatmap_onload_offset", beatmap_onload_offset);
    }
    FR_PTR_INFO("selected_song_code_start", selected_song_code_start);
    if (selected_song_code_start)
    {
        uintptr_t selected_song_offset = find_opcodes(selected_song_signature, selected_song_code_start, 0x100, 0x5A6);
        if (selected_song_offset)
            selected_song_ptr = *(uintptr_t *)(selected_song_code_start + selected_song_offset + 0x8);
        FR_PTR_INFO("selected_song_ptr", selected_song_ptr);
    }
    FR_PTR_INFO("audio_time_code_start", audio_time_code_start);
    if (audio_time_code_start)
    {
        uintptr_t audio_time_offset = find_opcodes(audio_time_signature, audio_time_code_start, 0x0, 0x5A6);
        if (audio_time_offset)
            audio_time_ptr = *(uintptr_t *)(audio_time_code_start + audio_time_offset - 0xA);
        FR_PTR_INFO("audio_time_ptr", audio_time_ptr);
    }
    FR_PTR_INFO("osu_manager_code_start", osu_manager_code_start);
    if (osu_manager_code_start)
    {
        uintptr_t osu_manager_offset = find_opcodes(osu_manager_signature, osu_manager_code_start, 0x0, 0x150);
        if (osu_manager_offset)
            osu_manager_ptr = *(uintptr_t *)(osu_manager_code_start + osu_manager_offset - 0x4);
        FR_PTR_INFO("osu_manager_ptr", osu_manager_ptr);
    }
    FR_PTR_INFO("binding_manager_code_start", binding_manager_code_start);
    if (binding_manager_code_start)
    {
        uintptr_t binding_manager_offset = find_opcodes(binding_manager_signature, binding_manager_code_start, 0x0, 0x100);
        if (binding_manager_offset)
        {
            uintptr_t unknown_ptr = binding_manager_code_start + binding_manager_offset + 0x6;
            if (internal_memory_read(g_process, unknown_ptr, &unknown_ptr))
                if (internal_memory_read(g_process, unknown_ptr, &unknown_ptr))
                    if (internal_memory_read(g_process, unknown_ptr + 0x8, &unknown_ptr))
                        binding_manager_ptr = unknown_ptr + 0x14;
        }
        FR_PTR_INFO("binding_manager_ptr", binding_manager_ptr);
    }
    FR_PTR_INFO("selected_replay_code_start", selected_replay_code_start);
    if (selected_replay_code_start)
    {
        selected_replay_offset = find_opcodes(selected_replay_signature, selected_replay_code_start, 0x200, 0x718);
        if (selected_replay_offset)
            selected_replay_hook_jump_back = selected_replay_code_start + selected_replay_offset + 0x7;
        FR_PTR_INFO("selected_replay_offset", selected_replay_offset);
    }
    if (osu_client_id_code_start)
    {
        __try
        {
            uintptr_t client_id_offset = find_opcodes(osu_client_id_function_signature, osu_client_id_code_start, 0x0, 0xBF);
            uintptr_t client_id_list = **(uintptr_t **)(osu_client_id_code_start + client_id_offset + sizeof(osu_client_id_function_signature));
            FR_PTR_INFO("client_id_list", client_id_list);
            uintptr_t client_id_array = *(uintptr_t *)(client_id_list + 0x4);
            FR_PTR_INFO("client_id_array", client_id_array);
            uint32_t strings_count = *(uint32_t *)(client_id_array + 0x4);
            for (uint32_t i = 0; i < strings_count; ++i)
            {
                uintptr_t string_ptr = *(uintptr_t *)(client_id_array + 0x8 + 0x4 * i);
                if (string_ptr != 0)
                {
                    uint32_t string_length = *(uint32_t *)(string_ptr + 0x4);
                    if (string_length == 32)
                    {
                        wchar_t *client_id_data = (wchar_t *)(string_ptr + 0x8);
                        int client_bytes_written = WideCharToMultiByte(CP_UTF8, 0, client_id_data, string_length, osu_client_id, 64, 0, 0);
                        osu_client_id[client_bytes_written] = '\0';
                        break;
                    }
                }
            }
        }
        __except(filter(GetExceptionCode(), GetExceptionInformation()))
        {
            FR_INFO_FMT("exception in try_find_hook_offsets: %s", "osu_client_id_code_start");
        }
    }
    FR_INFO_FMT("client_id: %s", osu_client_id);
    if (osu_username_code_start)
    {
        __try
        {
            uintptr_t username_offset = find_opcodes(osu_username_signature, osu_username_code_start, 0x20, 0x14D);
            if (username_offset)
            {
                    uintptr_t username_string = **(uintptr_t **)(osu_username_code_start + username_offset + sizeof(osu_username_signature));
                    uint32_t username_length = *(uint32_t *)(username_string + 0x4);
                    wchar_t *username_data = (wchar_t *)(username_string + 0x8);
                    int username_bytes_written = WideCharToMultiByte(CP_UTF8, 0, username_data, username_length, osu_username, 31, 0, 0);
                    osu_username[username_bytes_written] = '\0';
                }
            }
        __except(filter(GetExceptionCode(), GetExceptionInformation()))
        {
            FR_INFO_FMT("exception in try_find_hook_offsets: %s", "osu_username_code_start");
        }
    }
    FR_INFO_FMT("username: %s", osu_username);
    FR_PTR_INFO("window_manager_code_start", window_manager_code_start);
    if (window_manager_code_start)
    {
        window_manager_offset = find_opcodes(window_manager_signature, window_manager_code_start, 0x50, 0xC0A);
        if (window_manager_offset)
            window_manager_ptr = *(uintptr_t *)(window_manager_code_start + window_manager_offset + sizeof(window_manager_signature));
    }

    FR_PTR_INFO("score_multiplier_code_start", score_multiplier_code_start);
    if (score_multiplier_code_start)
    {
        score_multiplier_code_start += 0x2;
        score_multiplier_hook_jump_back = score_multiplier_code_start + 0x5;
    }
}

void init_hooks()
{
    HMODULE win32u = GetModuleHandle(L"win32u.dll");
    if (win32u != NULL)
    {
        nt_user_send_input_ptr = (uintptr_t)GetProcAddress(win32u, "NtUserSendInput");
        if (nt_user_send_input_ptr == NULL)
            FR_INFO("NtUserSendInput is null");
    }
    else
        FR_INFO("win32u.dll is null");

    if (!all_code_starts_found())
        scan_for_code_starts();

    try_find_hook_offsets();

    enable_nt_user_send_input_patch();

    init_input();

    if (ar_parameter.found)
    {
        ApproachRateHook1 = Hook<Detour32>(parse_beatmap_code_start + approach_rate_offsets[0], (BYTE *)set_approach_rate, 9);
        ApproachRateHook2 = Hook<Detour32>(parse_beatmap_code_start + approach_rate_offsets[1], (BYTE *)set_approach_rate, 9);
        ApproachRateHook3 = Hook<Detour32>(parse_beatmap_code_start + approach_rate_offsets[2], (BYTE *)set_approach_rate_2, 12);
        if (ar_parameter.lock)
            enable_ar_hooks();
    }

    if (cs_parameter.found)
    {
        CircleSizeHook1 = Hook<Detour32>(parse_beatmap_code_start + circle_size_offsets[0], (BYTE *)set_circle_size, 9);
        CircleSizeHook2 = Hook<Detour32>(parse_beatmap_code_start + circle_size_offsets[1], (BYTE *)set_circle_size, 9);
        CircleSizeHook3 = Hook<Detour32>(parse_beatmap_code_start + circle_size_offsets[2], (BYTE *)set_circle_size, 9);
        if (cs_parameter.lock)
            enable_cs_hooks();
    }

    if (od_parameter.found)
    {
        OverallDifficultyHook1 = Hook<Detour32>(parse_beatmap_code_start + overall_difficulty_offsets[0], (BYTE *)set_overall_difficulty, 9);
        OverallDifficultyHook2 = Hook<Detour32>(parse_beatmap_code_start + overall_difficulty_offsets[1], (BYTE *)set_overall_difficulty, 9);
        if (od_parameter.lock)
            enable_od_hooks();
    }

    if (beatmap_onload_offset)
    {
        BeatmapOnLoadHook = Hook<Detour32>(beatmap_onload_code_start + beatmap_onload_offset, (BYTE *)notify_on_beatmap_load, 6);
        if (cfg_replay_enabled || cfg_relax_lock || cfg_aimbot_lock)
            BeatmapOnLoadHook.Enable();
    }

    if (selected_replay_offset)
    {
        SelectedReplayHook = Hook<Detour32>(selected_replay_code_start + selected_replay_offset, (BYTE *)notify_on_select_replay, 7);
        if (cfg_replay_enabled)
            SelectedReplayHook.Enable();
    }

    if (score_multiplier_code_start)
    {
        ScoreMultiplierHook = Hook<Detour32>(score_multiplier_code_start, (BYTE *)set_score_multiplier, 5);
        if (cfg_score_multiplier_enabled)
            enable_score_multiplier_hooks();
    }
}

void enable_nt_user_send_input_patch()
{
    if (nt_user_send_input_ptr /* && *(uint8_t *)nt_user_send_input_ptr == (uint8_t)0xE9 */)
    {
        DWORD oldprotect;
        VirtualProtect((BYTE *)nt_user_send_input_ptr, 5, PAGE_EXECUTE_READWRITE, &oldprotect);
        nt_user_send_input_original_jmp_address = *(uintptr_t *)(nt_user_send_input_ptr + 0x1);
        *(uint8_t *)nt_user_send_input_ptr = (uint8_t)0xB8; // mov eax
        *(uintptr_t *)(nt_user_send_input_ptr + 0x1) = dispatch_table_id;
        VirtualProtect((BYTE *)nt_user_send_input_ptr, 5, oldprotect, &oldprotect);
    }
}

void disable_nt_user_send_input_patch()
{
    if (nt_user_send_input_ptr && nt_user_send_input_original_jmp_address)
    {
        DWORD oldprotect;
        VirtualProtect((BYTE *)nt_user_send_input_ptr, 5, PAGE_EXECUTE_READWRITE, &oldprotect);
        *(uint8_t *)nt_user_send_input_ptr = (uint8_t)0xE9;
        *(uintptr_t *)(nt_user_send_input_ptr + 0x1) = nt_user_send_input_original_jmp_address;
        VirtualProtect((BYTE *)nt_user_send_input_ptr, 5, oldprotect, &oldprotect);
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

void enable_notify_hooks()
{
    if (!cfg_relax_lock || !cfg_aimbot_lock || !cfg_replay_enabled)
    {
        BeatmapOnLoadHook.Enable();
    }
}

void disable_notify_hooks()
{
    if (!cfg_relax_lock && !cfg_aimbot_lock && !cfg_replay_enabled)
    {
        BeatmapOnLoadHook.Disable();
    }
}

void enable_replay_hooks()
{
    enable_notify_hooks();
    SelectedReplayHook.Enable();
}

void disable_replay_hooks()
{
    disable_notify_hooks();
    SelectedReplayHook.Disable();
}

void enable_score_multiplier_hooks()
{
    ScoreMultiplierHook.Enable();
}

void disable_score_multiplier_hooks()
{
    ScoreMultiplierHook.Disable();
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

__declspec(naked) void notify_on_beatmap_load()
{
    __asm {
        mov beatmap_loaded, 1
        mov eax, [esi+0x00000080]
        jmp [beatmap_onload_hook_jump_back]
    }
}

__declspec(naked) void notify_on_select_replay()
{
    __asm {
        mov eax, [esi+0x38]
        mov selected_replay_ptr, eax
        mov start_parse_replay, 1
        cmp dword ptr [eax+30], 0x00
        jmp [selected_replay_hook_jump_back]
    }
}

__declspec(naked) void set_score_multiplier()
{
    __asm {
        fld dword ptr [cfg_score_multiplier_value]
        cmp edx, 0x04
        jmp [score_multiplier_hook_jump_back]
    }
}

void destroy_hooks()
{
    SwapBuffersHook.Disable();
    if (ar_parameter.lock)  disable_ar_hooks();
    if (cs_parameter.lock)  disable_cs_hooks();
    if (od_parameter.lock)  disable_od_hooks();
    if (cfg_replay_enabled) SelectedReplayHook.Disable();
    if (cfg_replay_enabled || cfg_relax_lock || cfg_aimbot_lock) BeatmapOnLoadHook.Disable();
    if (cfg_score_multiplier_enabled) disable_score_multiplier_hooks();
    disable_nt_user_send_input_patch();
}
