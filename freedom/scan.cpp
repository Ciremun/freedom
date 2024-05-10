#include "scan.h"

#define PATTERN_SCAN(out, signature, start) if (!out) out = pattern::find<signature>({ start, signature.size() })

twglSwapBuffers wglSwapBuffersGateway;

uintptr_t beatmap_onload_code_start = 0;
uintptr_t beatmap_onload_offset = 0;
uintptr_t beatmap_onload_hook_jump_back = 0;

uintptr_t current_scene_code_start = 0;
uintptr_t current_scene_offset = 0;
Scene *current_scene_ptr = 0;
typedef void(*tSceneHook)();
tSceneHook o_scene_hook;
Hook<Trampoline32> SceneHook;

uintptr_t selected_song_code_start = 0;
uintptr_t selected_song_ptr = 0;

uintptr_t audio_time_code_start = 0;
uintptr_t audio_time_ptr = 0;

uintptr_t osu_manager_code_start = 0;
uintptr_t osu_manager_ptr = 0;

uintptr_t selected_mods_code_start = 0;
Mods *selected_mods_ptr = 0;

uintptr_t update_mods_code_start = 0;
uintptr_t update_mods_offset = 0;
typedef void(*tUpdateModsHook)();
tUpdateModsHook o_update_mods_hook;
Hook<Trampoline32> UpdateModsHook;

uintptr_t nt_user_send_input_ptr = 0;
uintptr_t nt_user_send_input_original_jmp_address = 0;
uintptr_t dispatch_table_id = 0x0000107F;
uintptr_t nt_user_send_input_dispatch_table_id_found = false;

uintptr_t osu_client_id_code_start = 0;
char osu_client_id[64] = {0};

uintptr_t osu_username_code_start = 0;
char osu_username[32] = {0};

float memory_scan_progress = .0f;

Hook<Trampoline32> SwapBuffersHook;
Hook<Detour32> BeatmapOnLoadHook;

uintptr_t selected_song_offset = 0;
uintptr_t audio_time_offset = 0;
uintptr_t osu_manager_offset = 0;
uintptr_t binding_manager_offset = 0;
uintptr_t client_id_offset = 0;
uintptr_t username_offset = 0;
uintptr_t check_timewarp_offset = 0;

CodeStart all_code_starts[] = {
    { .name = "Parse Beatmap",     .ptr = &parse_beatmap_code_start },
    { .name = "Beatmap Onload",    .ptr = &beatmap_onload_code_start },
    { .name = "Current Scene",     .ptr = &current_scene_code_start },
    { .name = "Selected Beatmap",  .ptr = &selected_song_code_start },
    { .name = "Audio Time",        .ptr = &audio_time_code_start },
    { .name = "Osu Manager",       .ptr = &osu_manager_code_start },
    { .name = "Binding Manager",   .ptr = &binding_manager_code_start },
    { .name = "Selected Replay",   .ptr = &selected_replay_code_start },
    { .name = "Osu Client ID",     .ptr = &osu_client_id_code_start },
    { .name = "Osu Username",      .ptr = &osu_username_code_start },
    { .name = "Window Manager",    .ptr = &window_manager_code_start },
    { .name = "Score Multiplier",  .ptr = &score_multiplier_code_start },
    { .name = "Check Flashlight",  .ptr = &check_flashlight_code_start },
    { .name = "Update Flashlight", .ptr = &update_flashlight_code_start },
    { .name = "Update Timing",     .ptr = &update_timing_code_start },
    { .name = "Set Playback Rate", .ptr = &set_playback_rate_code_start },
    { .name = "Check Timewarp",    .ptr = &check_timewarp_code_start },
    { .name = "Selected Mods",     .ptr = &selected_mods_code_start },
    { .name = "Update Mods",       .ptr = &update_mods_code_start },
    { .name = "Update Variables",  .ptr = &hom_update_vars_hidden_loc },
    { .name = "SendInput",         .ptr = &nt_user_send_input_dispatch_table_id_found },
};

inline bool all_code_starts_found()
{
    for (const auto &code_start : all_code_starts)
        if (!*code_start.ptr)
            return false;
    return true;
}

static inline bool some_feature_requires_update_mods_hook()
{
    return ar_parameter.lock || cs_parameter.lock || od_parameter.lock;
}

static inline bool some_feature_requires_notify_hooks()
{
    return cfg_relax_lock || cfg_aimbot_lock || cfg_replay_enabled || cfg_hidden_remover_enabled || cfg_flashlight_enabled;
}

static int filter(unsigned int code)
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

template <typename T>
static void try_(const char *name, T func)
{
    __try { func(); }
    __except (filter(GetExceptionCode())) {
        FR_INFO("there was an exception in '%s'", name);
    }
}

static inline uintptr_t GetModuleBaseAddress(const wchar_t* modName)
{
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, 0);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry))
        {
            do
            {
                if (!_wcsicmp(modEntry.szModule, modName))
                {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
    CloseHandle(hSnap);
    return modBaseAddr;
}

static inline bool is_dispatch_table_id(uint8_t *opcodes)
{
    return *opcodes == (uint8_t)0xB8 && opcodes[5] == (uint8_t)0xE9 &&
           (*(uintptr_t *)(opcodes + 0x6) + (uintptr_t)opcodes + 0x5 + 0x5) == (uintptr_t)(nt_user_send_input_ptr + 0x5);
}

static inline bool is_set_playback_rate(uint8_t *opcodes)
{
    // 55 8B EC 56 8B 35 ?? ?? ?? ?? 85 F6
    const uint8_t set_playback_rate_signature_first_part[] = {0x55, 0x8B, 0xEC, 0x56, 0x8B, 0x35};
    const uint8_t set_playback_rate_signature_second_part[] = {0x85, 0xF6, 0x75, 0x05, 0x5E, 0x5D, 0xC2, 0x08, 0x00, 0x33, 0xD2, 0x89, 0x15};
    return (memcmp(opcodes, set_playback_rate_signature_first_part, sizeof(set_playback_rate_signature_first_part)) == 0) && (memcmp(opcodes + 10, set_playback_rate_signature_second_part, sizeof(set_playback_rate_signature_second_part)) == 0);
}

static void scan_for_code_starts()
{
    prepare_methods();
    double s = ImGui::GetTime();
    int alignment = 8;
    _MEMORY_BASIC_INFORMATION mbi;
    for (uint8_t *p = (uint8_t *)GetModuleBaseAddress(L"osu!.exe"); VirtualQuery(p, &mbi, sizeof(mbi)); p += mbi.RegionSize)
    {
        if (mbi.State != MEM_COMMIT)
            continue;

        if (mbi.Protect != PAGE_EXECUTE_READWRITE)
        {
            if (!nt_user_send_input_dispatch_table_id_found && mbi.Protect == PAGE_EXECUTE_READ)
            {
                for (unsigned int idx = 0; idx != mbi.RegionSize / alignment; ++idx)
                {
                    uint8_t *opcodes = (uint8_t *)((uintptr_t)mbi.BaseAddress + idx * alignment);
                    if (is_dispatch_table_id(opcodes))
                    {
                        dispatch_table_id = *(uintptr_t *)(opcodes + 0x1);
                        nt_user_send_input_dispatch_table_id_found = true;
                        if (all_code_starts_found())
                        {
                            FR_INFO("Memory Scan Took: %lfs", ImGui::GetTime() - s);
                            memory_scan_progress = 1.f;
                            return;
                        }
                    }
                }
            }
            continue;
        }

        for (unsigned int idx = 0; idx != mbi.RegionSize / alignment; ++idx)
        {
            uint8_t *opcodes = (uint8_t *)((uintptr_t)mbi.BaseAddress + idx * alignment);
            memory_scan_progress = (uintptr_t)opcodes / (float)0x7FFFFFFF;

            PATTERN_SCAN(parse_beatmap_code_start,     parse_beatmap_func_sig,      opcodes);
            PATTERN_SCAN(beatmap_onload_code_start,    beatmap_onload_func_sig,     opcodes);
            PATTERN_SCAN(current_scene_code_start,     current_scene_func_sig,      opcodes);
            PATTERN_SCAN(selected_song_code_start,     selected_song_func_sig,      opcodes);
            PATTERN_SCAN(osu_manager_code_start,       osu_manager_func_sig,        opcodes);
            PATTERN_SCAN(binding_manager_code_start,   binding_manager_func_sig,    opcodes);
            PATTERN_SCAN(selected_replay_code_start,   selected_replay_func_sig,    opcodes);
            PATTERN_SCAN(osu_client_id_code_start,     osu_client_id_func_sig,      opcodes);
            PATTERN_SCAN(osu_username_code_start,      username_func_sig,           opcodes);
            PATTERN_SCAN(window_manager_code_start,    window_manager_func_sig,     opcodes);
            PATTERN_SCAN(score_multiplier_code_start,  score_multiplier_sig,        opcodes);
            PATTERN_SCAN(update_flashlight_code_start, update_flashlight_func_sig,  opcodes);
            PATTERN_SCAN(check_flashlight_code_start,  check_flashlight_func_sig,   opcodes);
            PATTERN_SCAN(update_timing_code_start,     update_timing_func_sig,      opcodes);
            PATTERN_SCAN(check_timewarp_code_start,    check_timewarp_func_sig,     opcodes);
            PATTERN_SCAN(selected_mods_code_start,     selected_mods_func_sig,      opcodes);
            PATTERN_SCAN(update_mods_code_start,       update_mods_func_sig,        opcodes);
            PATTERN_SCAN(hom_update_vars_hidden_loc,   hom_update_vars_hidden_sig,  opcodes);

            if (!set_playback_rate_code_start && is_set_playback_rate(opcodes))
            {
                set_playback_rate_code_start = (uintptr_t)opcodes;
                set_playback_rate_original_mov_addr = *(uintptr_t *)(opcodes + 0x6);
            }

            if (all_code_starts_found())
            {
                FR_INFO("Memory Scan Took: %lfs", ImGui::GetTime() - s);
                memory_scan_progress = 1.f;
                return;
            }
        }
    }
    FR_INFO("Memory Scan Took: %lfs", ImGui::GetTime() - s);
    memory_scan_progress = 1.f;
}

static void try_find_hook_offsets()
{
    if (parse_beatmap_code_start)
    {
        uint8_t *start = (uint8_t *)parse_beatmap_code_start;
        approach_rate_offsets[0] = pattern::find<approach_rate_sig>({(uint8_t *)start + 0x144A, 0x1CFF - 0x1000 });
        approach_rate_offsets[1] = pattern::find<approach_rate_sig>({(uint8_t *)start + 0x147A, 0x1CFF - 0x1000 });
        approach_rate_offsets[2] = pattern::find<approach_rate_sig_2>({ (uint8_t *)start + 0x1335, 0x1CFF - 0x1000 });
        circle_size_offsets[0] = pattern::find<circle_size_sig>({ (uint8_t *)start + 0x122D, 0x1CFF - 0x1000 });
        circle_size_offsets[1] = pattern::find<circle_size_sig>({ (uint8_t *)start + 0x1265, 0x1CFF - 0x1000 });
        circle_size_offsets[2] = pattern::find<circle_size_sig>({ (uint8_t *)start + 0x12B9, 0x1CFF - 0x1000 });
        overall_difficulty_offsets[0] = pattern::find<overall_difficulty_sig>({ (uint8_t *)start + 0x1305, 0x1CFF - 0x1000 });
        overall_difficulty_offsets[1] = pattern::find<overall_difficulty_sig>({ (uint8_t *)start + 0x1345, 0x1CFF - 0x1000 });
        ar_hook_jump_back = approach_rate_offsets[1] + 0x9;
        cs_hook_jump_back = circle_size_offsets[0] + 0x9;
        od_hook_jump_back = overall_difficulty_offsets[1] + 0x9;
        ar_parameter.found = approach_rate_offsets[1] > 0 && approach_rate_offsets[2] > 0;
        cs_parameter.found = circle_size_offsets[2] > 0;
        od_parameter.found = overall_difficulty_offsets[1] > 0;
    }
    if (current_scene_code_start)
    {
        current_scene_offset = pattern::find<current_scene_sig>({ (uint8_t *)current_scene_code_start, 0x800 + 0x18});
        current_scene_ptr = *(Scene **)(current_scene_offset + 0xF + 0x1);
    }
    if (beatmap_onload_code_start)
    {
        beatmap_onload_offset = pattern::find<beatmap_onload_sig>({ (uint8_t *)beatmap_onload_code_start, 0x300 + 0x50});
        if (beatmap_onload_offset)
            beatmap_onload_hook_jump_back = beatmap_onload_offset + 0x6;
    }
    if (selected_song_code_start)
    {
        selected_song_offset = pattern::find<selected_song_sig>({ (uint8_t *)selected_song_code_start, 0x5A6 + 0x100});
        if (selected_song_offset)
            selected_song_ptr = *(uintptr_t *)(selected_song_offset + 0x8);

        // NOTE(Ciremun): same signature
        audio_time_code_start = selected_song_code_start;
        audio_time_offset = pattern::find<audio_time_sig>({ (uint8_t *)audio_time_code_start, 0x5A6});
        if (audio_time_offset)
            audio_time_ptr = *(uintptr_t *)(audio_time_offset - 0xA);
    }
    if (osu_manager_code_start)
    {
        osu_manager_offset = pattern::find<osu_manager_sig>({ (uint8_t *)osu_manager_code_start, 0x150});
        if (osu_manager_offset)
            osu_manager_ptr = *(uintptr_t *)(osu_manager_offset - 0x4);
    }
    if (binding_manager_code_start)
    {
        binding_manager_offset = pattern::find<binding_manager_sig>({ (uint8_t *)binding_manager_code_start, 0x100});
        if (binding_manager_offset)
        {
            uintptr_t unknown_ptr = binding_manager_offset + 0x6;
            if (internal_memory_read(g_process, unknown_ptr, &unknown_ptr))
                if (internal_memory_read(g_process, unknown_ptr, &unknown_ptr))
                    if (internal_memory_read(g_process, unknown_ptr + 0x8, &unknown_ptr))
                        binding_manager_ptr = unknown_ptr + 0x14;
        }
    }
    if (selected_replay_code_start)
    {
        selected_replay_offset = pattern::find<selected_replay_sig>({ (uint8_t *)selected_replay_code_start, 0x718 + 0x200});
        if (selected_replay_offset)
            selected_replay_hook_jump_back = selected_replay_offset + 0x7;
    }
    if (osu_client_id_code_start)
    {
        try_("osu_client_id_code_start", [](){
            client_id_offset = pattern::find<osu_client_id_func_sig>({ (uint8_t *)osu_client_id_code_start, 0xBF});
            uintptr_t client_id_list = **(uintptr_t **)(client_id_offset + osu_client_id_func_sig.size());
            uintptr_t client_id_array = *(uintptr_t *)(client_id_list + 0x4);
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
        });
    }
    if (osu_username_code_start)
    {
        try_("osu_username_code_start", [](){
            username_offset = pattern::find<osu_username_sig>({ (uint8_t *)osu_username_code_start, 0x14D + 0x20});
            if (username_offset)
            {
                uintptr_t username_string = **(uintptr_t **)(username_offset + osu_username_sig.size());
                uint32_t username_length = *(uint32_t *)(username_string + 0x4);
                wchar_t *username_data = (wchar_t *)(username_string + 0x8);
                int username_bytes_written = WideCharToMultiByte(CP_UTF8, 0, username_data, username_length, osu_username, 31, 0, 0);
                osu_username[username_bytes_written] = '\0';
            }
        });
    }
    if (window_manager_code_start)
    {
        window_manager_offset = pattern::find<window_manager_sig>({ (uint8_t *)window_manager_code_start, 0x55});
        if (window_manager_offset)
            window_manager_ptr = *(uintptr_t *)(window_manager_offset + 0x2);
    }

    if (score_multiplier_code_start)
    {
        score_multiplier_offset = score_multiplier_code_start + 0xA;
        score_multiplier_hook_jump_back = score_multiplier_offset + 0x5;
    }

    if (check_timewarp_code_start)
    {
        // D9 E8 DE F1 DE C9
        check_timewarp_offset = pattern::find<check_timewarp_sig>({ (uint8_t *)check_timewarp_code_start, 0x16EA + 0x1000});
        check_timewarp_hook_1 = (uintptr_t)(check_timewarp_offset - 0x24);
        check_timewarp_hook_2 = (uintptr_t)(check_timewarp_sig.size() + check_timewarp_offset + 0x5);
        check_timewarp_hook_1_jump_back = check_timewarp_hook_1 + 0x6;
        check_timewarp_hook_2_jump_back = check_timewarp_hook_2 + 0x6;
    }

    // if (update_timing_code_start)
    // {
    //     uintptr_t update_timing_ptr_1_offset = pattern::find<update_timing_sig>({ (uint8_t *)update_timing_code_start, 0x1F4 + 0x100});
    //     update_timing_ptr_1 = *(uintptr_t *)(update_timing_code_start + update_timing_ptr_1_offset + update_timing_sig.size());
    //     uintptr_t offset_of_something_in_between = pattern::find<update_timing_sig_2>({ (uint8_t *)update_timing_code_start, 0x280 + 0x1F4});
    //     update_timing_ptr_2 = *(uintptr_t *)(update_timing_code_start + offset_of_something_in_between - 0x24);
    //     update_timing_ptr_3 = *(uintptr_t *)(update_timing_code_start + offset_of_something_in_between - 0x4);
    //     update_timing_ptr_4 = *(uintptr_t *)(update_timing_code_start + offset_of_something_in_between + 0x39);
    // }

    if (set_playback_rate_code_start)
    {
        set_playback_rate_jump_back = set_playback_rate_code_start + 0xA;
    }

    if (selected_mods_code_start)
    {
        selected_mods_ptr = *(Mods **)(selected_mods_code_start + 0x9);
    }

    if (update_mods_code_start)
    {
        update_mods_offset = pattern::find<update_mods_sig>({ (uint8_t *)update_mods_code_start, 0xC1});
        if (update_mods_offset && !selected_mods_ptr)
        {
            selected_mods_ptr = *(Mods **)(update_mods_offset + 0x7);
        }
    }
}

static inline void init_nt_user_send_input_patch()
{
    HMODULE win32u = GetModuleHandle(L"win32u.dll");
    if (win32u != NULL)
    {
        nt_user_send_input_ptr = (uintptr_t)GetProcAddress(win32u, "NtUserSendInput");
        if (nt_user_send_input_ptr == NULL)
            FR_ERROR("NtUserSendInput is null");
    }
    else
        FR_ERROR("win32u.dll is null");
}

static inline void init_hooks_wrapper()
{
    init_nt_user_send_input_patch();
    try_("init_clrhost", [](){ init_clrhost(); });
    try_("scan_for_code_starts", [](){ scan_for_code_starts(); });
    try_("try_find_hook_offsets", [](){ try_find_hook_offsets(); });

    for (const auto &code_start : all_code_starts)
        if (!*code_start.ptr)
            FR_ERROR("'%s' wasn't found", code_start.name);

    if (scene_is_game(current_scene_ptr))
        enable_nt_user_send_input_patch();

    bool notify_hooks_required = some_feature_requires_notify_hooks();
    if (beatmap_onload_offset)
    {
        BeatmapOnLoadHook = Hook<Detour32>(beatmap_onload_offset, (BYTE *)notify_on_beatmap_load, 6);
        if (notify_hooks_required)
            BeatmapOnLoadHook.Enable();
    }
    if (current_scene_offset)
    {
        SceneHook = Hook<Trampoline32>(current_scene_offset + 0xF, (BYTE *)notify_on_scene_change, (BYTE *)&o_scene_hook, 5);
        if (notify_hooks_required)
            SceneHook.Enable();
    }
    if (update_mods_offset)
    {
        UpdateModsHook = Hook<Trampoline32>(update_mods_offset, (BYTE *)notify_on_update_mods, (BYTE *)&o_update_mods_hook, 5);
        if (some_feature_requires_update_mods_hook())
            UpdateModsHook.Enable();
    }

    init_input();
    init_difficulty();
    init_timewarp();
    init_score_multiplier();
    init_discord_rpc();
    init_replay();
    init_unmod_flashlight();
    init_unmod_hidden();
}

void init_hooks()
{
    try_("init_hooks_wrapper", [](){ init_hooks_wrapper(); });
}

void enable_nt_user_send_input_patch()
{
    if (nt_user_send_input_ptr && *(uint8_t *)nt_user_send_input_ptr == (uint8_t)0xE9)
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

void enable_notify_hooks()
{
    BeatmapOnLoadHook.Enable();
    SceneHook.Enable();
}

void disable_notify_hooks()
{
    if (!some_feature_requires_notify_hooks())
    {
        BeatmapOnLoadHook.Disable();
        SceneHook.Disable();
    }
}

static inline void disable_notify_hooks_force()
{
    BeatmapOnLoadHook.Disable();
    SceneHook.Disable();
}

void enable_update_mods_hook()
{
    UpdateModsHook.Enable();
}

void disable_update_mods_hook()
{
    if (!some_feature_requires_update_mods_hook())
        UpdateModsHook.Disable();
}

static inline void disable_update_mods_hook_force()
{
    UpdateModsHook.Disable();
}

__declspec(naked) void notify_on_beatmap_load()
{
    __asm {
        mov beatmap_loaded, 1
        sete dl
        mov [eax+0x39], dl
        jmp [beatmap_onload_hook_jump_back]
    }
}

__declspec(naked) void notify_on_scene_change()
{
    static Scene new_scene;

    __asm {
        mov new_scene, eax
        push eax
    }

    if (new_scene == Scene::GAME)
    {
        __asm {
            call enable_nt_user_send_input_patch
        }
    }
    else
    {
        __asm {
            mov current_beatmap.ready, 0
            call disable_nt_user_send_input_patch
        }
    }

    __asm {
        pop eax
        jmp o_scene_hook
    }
}

__declspec(naked) void notify_on_update_mods()
{
    __asm {
        mov mods_updated, 1
        jmp o_update_mods_hook
    }
}

void destroy_hooks_except_swap()
{
    disable_ar_hooks();
    disable_cs_hooks();
    disable_od_hooks();
    disable_notify_hooks_force();
    disable_update_mods_hook_force();
    disable_replay_hooks();
    disable_flashlight_hooks();
    disable_score_multiplier_hooks();
    disable_drpc_hooks();
    disable_timewarp_hooks();
    disable_hidden_remover_hooks();
    disable_nt_user_send_input_patch();
}

void destroy_hooks()
{
    SwapBuffersHook.Disable();
    destroy_hooks_except_swap();
}
