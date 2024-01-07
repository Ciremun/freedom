#include "ui.h"

ImFont *font = 0;
char song_name_u8[256] = "Freedom " FR_VERSION " is Loading!";
bool show_debug_log_window = false;

HHOOK oWndProc;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall WndProc(int code, WPARAM wparam, LPARAM lparam)
{
    if (code > 0)
        return CallNextHookEx(oWndProc, code, wparam, lparam);

    MSG *message = (MSG *)lparam;

    if (wparam == PM_REMOVE)
    {
        if (ImGui_ImplWin32_WndProcHandler(message->hwnd, message->message, message->wParam, message->lParam))
        {
            message->message = WM_NULL;
            return 1;
        }
    }

    if (message->message == WM_LBUTTONUP && !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemFocused() && !ImGui::IsAnyItemActive())
    {
        ImGui::GetIO().MouseDrawCursor = false;
        cfg_mod_menu_visible = false;
    }

    if (cfg_mod_menu_visible && ((message->message >= WM_MOUSEFIRST && message->message <= WM_MOUSELAST) || message->message == WM_CHAR))
    {
        message->message = WM_NULL;
        return 1;
    }

    return CallNextHookEx(oWndProc, code, wparam, lparam);
}

inline void init_imgui_styles()
{
    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.FrameRounding = 3.f;
    style.Colors[ImGuiCol_TitleBgActive] = PURPLE;
    style.Colors[ImGuiCol_Button] = PURPLE;
    style.Colors[ImGuiCol_ButtonHovered] = MAGENTA;
    style.Colors[ImGuiCol_ButtonActive] = MAGENTA;
    style.Colors[ImGuiCol_PopupBg] = BLACK;
    style.Colors[ImGuiCol_MenuBarBg] = BLACK;
    style.Colors[ImGuiCol_Header] = PURPLE;
    style.Colors[ImGuiCol_HeaderHovered] = MAGENTA;
    style.Colors[ImGuiCol_HeaderActive] = MAGENTA;
    style.Colors[ImGuiCol_FrameBg] = PURPLE;
    style.Colors[ImGuiCol_FrameBgHovered] = PURPLE;
    style.Colors[ImGuiCol_FrameBgActive] = MAGENTA;
    style.Colors[ImGuiCol_SliderGrab] = BLACK_TRANSPARENT;
    style.Colors[ImGuiCol_SliderGrabActive] = BLACK_TRANSPARENT;
    style.Colors[ImGuiCol_CheckMark] = WHITE;
    style.Colors[ImGuiCol_PlotHistogram] = MAGENTA;
    style.Colors[ImGuiCol_ResizeGrip] = PURPLE;
    style.Colors[ImGuiCol_ResizeGripHovered] = MAGENTA;
    style.Colors[ImGuiCol_ResizeGripActive] = BLACK_TRANSPARENT;
}

inline void init_imgui_fonts()
{
    ImGuiIO &io = ImGui::GetIO();
    ImFontConfig config;
    config.OversampleH = config.OversampleV = 1;
    config.PixelSnapH = true;
    config.GlyphRanges = io.Fonts->GetGlyphRangesCyrillic();

    for (int size = 40; size >= 10; size -= 2)
    {
        config.SizePixels = size;
        ImFont *f = io.Fonts->AddFontFromMemoryCompressedBase85TTF(victor_mono_font_compressed_data_base85, size, &config);
        if (size == cfg_font_size)
            font = f;
    }
}

void init_ui(IDirect3DDevice9* pDevice)
{
    oWndProc = SetWindowsHookExA(WH_GETMESSAGE, &WndProc, GetModuleHandleA(nullptr), GetCurrentThreadId());

    IMGUI_CHECKVERSION();
    ImGuiContext* ctx = ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    ctx->SettingsHandlers.clear();

    set_imgui_ini_handler();
    io.IniFilename = get_imgui_ini_filename(g_module);

    ImGui::LoadIniSettingsFromDisk(io.IniFilename);

    init_imgui_styles();
    init_imgui_fonts();

    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX9_Init(pDevice);
}

void init_ui()
{
    oWndProc = SetWindowsHookExA(WH_GETMESSAGE, &WndProc, GetModuleHandleA(nullptr), GetCurrentThreadId());

    IMGUI_CHECKVERSION();
    ImGuiContext* ctx = ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    ctx->SettingsHandlers.clear();

    set_imgui_ini_handler();
    io.IniFilename = get_imgui_ini_filename(g_module);

    ImGui::LoadIniSettingsFromDisk(io.IniFilename);

    init_imgui_styles();
    init_imgui_fonts();

    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplOpenGL3_Init();
}

void update_ui()
{
    if (selected_song_ptr)
    {
        uintptr_t song_str_ptr = 0;
        if (internal_memory_read(g_process, selected_song_ptr, &song_str_ptr))
        {
            song_str_ptr += 0x80;
            static uintptr_t prev_song_str_ptr = 0;
            if (song_str_ptr != prev_song_str_ptr)
            {
                uintptr_t song_str = 0;
                if (internal_memory_read(g_process, song_str_ptr, &song_str))
                {
                    song_str += 0x4;
                    uint32_t song_str_length = 0;
                    if (internal_memory_read(g_process, song_str, &song_str_length))
                    {
                        song_str += 0x4;
                        int bytes_written = WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)song_str, song_str_length, song_name_u8, 255, 0, 0);
                        song_name_u8[bytes_written] = '\0';
                    }
                }
            }
            prev_song_str_ptr = song_str_ptr;
        }
    }

    ImGui::PushFont(font);

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
    ImGui::Begin("Freedom", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("%s", song_name_u8);

    if (memory_scan_progress < .99f)
    {
        static char overlay_buf[32] = {0};
        ImFormatString(overlay_buf, IM_ARRAYSIZE(overlay_buf), prepared_methods ? "Memory Scan: %.0f%%" : "Preparing Methods: %.0f%%", memory_scan_progress * 100 + 0.01f);
        ImGui::ProgressBar(memory_scan_progress, ImVec2(.0f, .0f), overlay_buf);
    }

    ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetWindowHeight()), ImGuiCond_Appearing);
    if (ImGui::BeginPopupContextItem("##settings"))
    {
        static MenuTab selected_tab = MenuTab::Difficulty;

        const auto update_tab = [](const char *tab_name, MenuTab tab_type)
        {
            if (ImGui::Selectable(tab_name, selected_tab == tab_type, ImGuiSelectableFlags_DontClosePopups))
            {
                selected_tab = tab_type;
                ImGui::SetNextWindowFocus();
            }
        };

        const auto inactive_tab = [](const char *tab_name)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ITEM_UNAVAILABLE);
            ImGui::Selectable(tab_name, false, ImGuiSelectableFlags_DontClosePopups);
            ImGui::PopStyleColor();
        };

        update_tab("Difficulty", MenuTab::Difficulty);

        beatmap_onload_offset ? update_tab("Relax",  MenuTab::Relax)  : inactive_tab("Relax");
        beatmap_onload_offset ? update_tab("Aimbot", MenuTab::Aimbot) : inactive_tab("Aimbot");
        set_playback_rate_code_start ? update_tab("Timewarp", MenuTab::Timewarp) : inactive_tab("Timewarp");
        selected_replay_offset ? update_tab("Replay", MenuTab::Replay) : inactive_tab("Replay");

        update_tab("Mods", MenuTab::Mods);
        update_tab("Misc", MenuTab::Misc);
        update_tab("About", MenuTab::About);
        update_tab("Debug", MenuTab::Debug);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(540.0f, 0.0f));
        if (selected_tab == MenuTab::Debug || selected_tab == MenuTab::Misc || selected_tab == MenuTab::Relax)
            ImGui::SetNextWindowSize(ImVec2(.0f, .0f), ImGuiCond_Always);
        else
            ImGui::SetNextWindowSize(ImVec2(.0f, ImGui::GetWindowHeight()), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth(), ImGui::GetWindowPos().y), ImGuiCond_Always);
        ImGui::Begin("##tab_content", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
        ImGui::PopStyleVar();
        if (selected_tab == MenuTab::Difficulty)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(7.5f, 7.5f));
            parameter_slider(selected_song_ptr, &ar_parameter);
            parameter_slider(selected_song_ptr, &cs_parameter);
            parameter_slider(selected_song_ptr, &od_parameter);
            ImGui::PopStyleVar();
        }
        if (selected_tab == MenuTab::Relax)
        {
            if (ImGui::Checkbox("Enable", &cfg_relax_lock))
            {
                cfg_relax_lock ? enable_notify_hooks() : disable_notify_hooks();
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            ImGui::PushItemWidth(ImGui::CalcTextSize("X").x * 1.85f);
            ImGui::InputText("Left Click",  left_click,  2, ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_AutoSelectAll);
            ImGui::InputText("Right Click", right_click, 2, ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_AutoSelectAll);
            ImGui::PopItemWidth();
            ImGui::Dummy(ImVec2(.0f, 5.f));
            if (ImGui::RadioButton("SingleTap", &cfg_relax_style, 's'))
            {
                FR_INFO("SingleTap Mode");
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Alternate", &cfg_relax_style, 'a'))
            {
                FR_INFO("Alternate Mode");
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::Dummy(ImVec2(.0f, 5.f));
            if (ImGui::Checkbox("Variable Unstable Rate", &cfg_relax_checks_od))
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            ImGui::Dummy(ImVec2(.0f, 5.f));
            ImGui::Checkbox("Debug", &debug_relax);
        }
        if (selected_tab == MenuTab::Aimbot)
        {
            if (ImGui::Checkbox("Enable", &cfg_aimbot_lock))
            {
                cfg_aimbot_lock ? enable_notify_hooks() : disable_notify_hooks();
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            ImGui::SliderFloat("##fraction_modifier", &cfg_fraction_modifier, 0.001f, 0.5f, "Cursor Speed: %.3f");
            if (ImGui::IsItemDeactivatedAfterEdit())
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            ImGui::Dummy(ImVec2(.0f, .5f));
            ImGui::SliderInt("##spins_per_minute", &cfg_spins_per_minute, 0, 477, "Spins Per Minute: %d");
            if (ImGui::IsItemDeactivatedAfterEdit())
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing());
        }
        if (selected_tab == MenuTab::Timewarp)
        {
            if (ImGui::Checkbox("Enable", &cfg_timewarp_enabled))
            {
                cfg_timewarp_enabled ? enable_timewarp_hooks() : disable_timewarp_hooks();
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            static double p_min = 0.1;
            static double p_max = 2.0;
            static double timewarp_playback_rate = cfg_timewarp_playback_rate / 100.0;
            if (ImGui::SliderScalar("##timewarp_scale", ImGuiDataType_Double, &timewarp_playback_rate, &p_min, &p_max, "Timewarp Scale: %.2lf"))
                cfg_timewarp_playback_rate = timewarp_playback_rate * 100.0;
            if (ImGui::IsItemDeactivatedAfterEdit())
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        }
        if (selected_tab == MenuTab::Replay)
        {
            ImGui::Text("%s", current_replay.song_name_u8);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("Selected Replay");
            ImGui::Text("%s - %.2f%% - %ux - %s", current_replay.author, current_replay.accuracy, current_replay.combo, current_replay.mods);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("Player, Accuracy, Mods");
            ImGui::Dummy(ImVec2(.0f, 2.f));
            if (ImGui::Checkbox("Enable", &cfg_replay_enabled))
            {
                cfg_replay_enabled ? enable_replay_hooks() : disable_replay_hooks();
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("Usage: Open Replay Preview in-game to Select a Replay");
            ImGui::SameLine(210.0f);
            if (!cfg_replay_enabled)
            {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleColor(ImGuiCol_Text, ITEM_DISABLED);
            }
            if (ImGui::Checkbox("Hardrock", &cfg_replay_hardrock))         current_replay.toggle_hardrock();
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("Convert Replay to/from Hardrock");
            ImGui::Dummy(ImVec2(.0f, 2.f));
            if (ImGui::Checkbox("Replay Aim", &cfg_replay_aim))            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("Aim According to Replay Data");
            ImGui::SameLine(210.0f);
            if (ImGui::Checkbox("Replay Keys", &cfg_replay_keys))          ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("Press Keys According to Replay Data");
            if (!cfg_replay_enabled)
            {
                ImGui::PopStyleColor();
                ImGui::PopItemFlag();
            }
        }
        if (selected_tab == MenuTab::Mods)
        {
            if (ImGui::Checkbox("Unmod Flashlight", &cfg_flashlight_enabled))
            {
                cfg_flashlight_enabled ? enable_flashlight_hooks() : disable_flashlight_hooks();
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::Dummy(ImVec2(.0f, 5.f));
            if (ImGui::Checkbox("Unmod Hidden", &cfg_hidden_remover_enabled))
            {
                cfg_hidden_remover_enabled ? enable_hidden_remover_hooks() : disable_hidden_remover_hooks();
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::Dummy(ImVec2(.0f, 5.f));
            ImGui::Text("Score Multiplier");
            ImGui::Dummy(ImVec2(.0f, 5.f));
            ImGui::PushID(70);
            if (ImGui::Checkbox("Enable", &cfg_score_multiplier_enabled))
            {
                cfg_score_multiplier_enabled ? enable_score_multiplier_hooks() : disable_score_multiplier_hooks();
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::PopID();
            ImGui::Dummy(ImVec2(.0f, 5.f));
            if (!cfg_score_multiplier_enabled)
            {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleColor(ImGuiCol_Text, ITEM_DISABLED);
            }
            ImGui::SliderFloat("##score_multiplier", &cfg_score_multiplier_value, .0f, 100.f, "Score Multiplier: %.0f");
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("Hold Ctrl To Set a Custom Value");
            if (ImGui::IsItemDeactivatedAfterEdit())
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            if (!cfg_score_multiplier_enabled)
            {
                ImGui::PopStyleColor();
                ImGui::PopItemFlag();
            }
        }
        if (selected_tab == MenuTab::Misc)
        {
            ImGui::Text("Discord RPC Settings");
            ImGui::Dummy(ImVec2(.0f, 5.f));

            if (ImGui::Checkbox("Enable", &cfg_discord_rich_presence_enabled))
            {
                cfg_discord_rich_presence_enabled ? enable_discord_rich_presence_hooks() : disable_discord_rich_presence_hooks();
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }

            ImGui::Dummy(ImVec2(.0f, 5.f));

            if (!cfg_discord_rich_presence_enabled)
            {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleColor(ImGuiCol_Text, ITEM_DISABLED);
            }

            static char discord_rich_presence_state[512] = {0};
            if (ImGui::InputTextEx("##rpc_state", "State", discord_rich_presence_state, 512, ImVec2(0, 0), ImGuiInputTextFlags_None))
            {
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
                static wchar_t discord_rich_presence_state_wchar[512] = {0};
                invoke_csharp_method(L"Freedom.Utils", L"FreeCSharpString", discord_rich_presence_state_wchar);
                int wchars_count = MultiByteToWideChar(CP_UTF8, 0, discord_rich_presence_state, -1, NULL, 0);
                int bytes_written = MultiByteToWideChar(CP_UTF8, 0, discord_rich_presence_state, -1, discord_rich_presence_state_wchar, wchars_count);
                discord_rich_presence_state_wchar[bytes_written] = '\0';
                VARIANT v = invoke_csharp_method(L"Freedom.Utils", L"GetCSharpStringPtr", discord_rich_presence_state_wchar);
                if (variant_ok(&v))
                    discord_rich_presence_state_string_ptr = v.intVal;
            }

            static char discord_rich_presence_large_text[512] = {0};
            if (ImGui::InputTextEx("##rpc_large_text", "Large Text", discord_rich_presence_large_text, 512, ImVec2(0, 0), ImGuiInputTextFlags_None))
            {
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
                static wchar_t discord_rich_presence_large_text_wchar[512] = {0};
                invoke_csharp_method(L"Freedom.Utils", L"FreeCSharpString", discord_rich_presence_large_text_wchar);
                int wchars_count = MultiByteToWideChar(CP_UTF8, 0, discord_rich_presence_large_text, -1, NULL, 0);
                int bytes_written = MultiByteToWideChar(CP_UTF8, 0, discord_rich_presence_large_text, -1, discord_rich_presence_large_text_wchar, wchars_count);
                discord_rich_presence_large_text_wchar[bytes_written] = '\0';
                VARIANT v = invoke_csharp_method(L"Freedom.Utils", L"GetCSharpStringPtr", discord_rich_presence_large_text_wchar);
                if (variant_ok(&v))
                    discord_rich_presence_large_text_string_ptr = v.intVal;
            }

            static char discord_rich_presence_small_text[512] = {0};
            if (ImGui::InputTextEx("##rpc_small_text", "Small Text", discord_rich_presence_small_text, 512, ImVec2(0, 0), ImGuiInputTextFlags_None))
            {
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
                static wchar_t discord_rich_presence_small_text_wchar[512] = {0};
                invoke_csharp_method(L"Freedom.Utils", L"FreeCSharpString", discord_rich_presence_small_text_wchar);
                int wchars_count = MultiByteToWideChar(CP_UTF8, 0, discord_rich_presence_small_text, -1, NULL, 0);
                int bytes_written = MultiByteToWideChar(CP_UTF8, 0, discord_rich_presence_small_text, -1, discord_rich_presence_small_text_wchar, wchars_count);
                discord_rich_presence_small_text_wchar[bytes_written] = '\0';
                VARIANT v = invoke_csharp_method(L"Freedom.Utils", L"GetCSharpStringPtr", discord_rich_presence_small_text_wchar);
                if (variant_ok(&v))
                    discord_rich_presence_small_text_string_ptr = v.intVal;
            }

            if (!cfg_discord_rich_presence_enabled)
            {
                ImGui::PopStyleColor();
                ImGui::PopItemFlag();
            }

            ImGui::Dummy(ImVec2(.0f, 10.f));

            static char preview_font_size[16] = {0};
            stbsp_snprintf(preview_font_size, 16, "Font Size: %dpx", (int)font->ConfigData->SizePixels);
            if (ImGui::BeginCombo("##font_size", preview_font_size, ImGuiComboFlags_HeightLargest))
            {
                const ImGuiIO& io = ImGui::GetIO();
                for (const auto &f : io.Fonts->Fonts)
                {
                    char font_size[8] = {0};
                    stbsp_snprintf(font_size, 4, "%d", (int)f->ConfigData->SizePixels);
                    const bool is_selected = f == font;
                    if (ImGui::Selectable(font_size, is_selected))
                    {
                        font = f;
                        cfg_font_size = (int)f->ConfigData->SizePixels;
                        ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
                    }
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::Dummy(ImVec2(.0f, 5.f));

            static bool nt_user_send_input_patched = true;
            if (ImGui::Checkbox("Disable NtUserSendInput Check", &nt_user_send_input_patched))
                nt_user_send_input_patched ? enable_nt_user_send_input_patch() : disable_nt_user_send_input_patch();
            ImGui::Dummy(ImVec2(.0f, 5.f));
            if (ImGui::Button("Unload DLL"))
                unload_dll();
            bool all_found = all_code_starts_found();
            if (all_found)
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            if (ImGui::Button("Rescan Memory"))
                CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)init_hooks, 0, 0 ,0));
            if (all_found)
                ImGui::PopItemFlag();
        }
        if (selected_tab == MenuTab::About)
        {
            ImGui::Text("Ciremun's Freedom " FR_VERSION);
            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            ImGui::Text("Special Thanks to Maple Syrup");
            ImGui::Text("@mrflashstudio");
            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            ImGui::PushItemWidth(ImGui::CalcTextSize("Ciremun").x + 10.f);
            ImGui::InputText(" - Discord", (char *)"Ciremun", 8, ImGuiInputTextFlags_ReadOnly);
            ImGui::PopItemWidth();
            ImGui::Dummy(ImVec2(0.0f, 2.5f));
            ImGui::PushItemWidth(ImGui::CalcTextSize("https://discord.gg/FyCrwed6jv").x + 10.f);
            ImGui::InputText(" - Discord Server ", (char *)"https://discord.gg/FyCrwed6jv", 29, ImGuiInputTextFlags_ReadOnly);
            ImGui::PopItemWidth();
        }
        if (selected_tab == MenuTab::Debug)
        {
            ImGui::Checkbox("Show Debug Log", &show_debug_log_window);
            if (ImGui::CollapsingHeader("Game", ImGuiTreeNodeFlags_None))
            {
                ImGui::Text("Audio Time: %d", audio_time_ptr ? *(int32_t *)audio_time_ptr : 0);
                const auto scene_ptr_to_str = [](Scene *s){
                    if (!s) return "Unknown";
                    Scene scene = *s;
                    switch (scene)
                    {
                        case Scene::MAIN_MENU: return "Main Menu";
                        case Scene::EDITOR: return "Editor";
                        case Scene::GAME: return "Game";
                        case Scene::EXIT: return "Exit";
                        case Scene::EDITOR_BEATMAP_SELECT: return "Editor Beatmap Select";
                        case Scene::BEATMAP_SELECT: return "Beatmap Select";
                        case Scene::BEATMAP_SELECT_DRAWINGS: return "Beatmap Select Drawings";
                        case Scene::REPLAY_PREVIEW: return "Replay Preview";
                        default:
                            return "Unknown";
                    }
                };
                ImGui::Text("Current Scene: %s", scene_ptr_to_str(current_scene_ptr));
                ImGui::Text("Replay Mode: %s", is_replay_mode(osu_manager_ptr) ? "Yes" : "No");
                ImGui::Text("Pre-Jitting Mode: %s", prepared_all_methods ? "All Methods" : "Fast");
            }
            if (ImGui::CollapsingHeader("Account Info", ImGuiTreeNodeFlags_None))
            {
                ImGui::Text("Client ID: \n%s", osu_client_id);
                ImGui::Text("Username: %s", osu_username);
            }
            if (ImGui::CollapsingHeader("Playfield", ImGuiTreeNodeFlags_None))
            {
                ImGui::Text("window_size: %f %f", window_size.x, window_size.y);
                ImGui::Text("playfield_size: %f %f", playfield_size.x, playfield_size.y);
                ImGui::Text("playfield_position: %f %f", playfield_position.x, playfield_position.y);
            }
            if (ImGui::CollapsingHeader("Pointers", ImGuiTreeNodeFlags_None))
            {
                ImGui::Text("binding_manager_ptr: %08X", binding_manager_ptr);
                ImGui::Text("osu_manager_ptr: %08X", osu_manager_ptr);
                ImGui::Text("selected_replay_ptr: %08X", selected_replay_ptr);
                ImGui::Text("selected_song_ptr: %08X", selected_song_ptr);
                ImGui::Text("update_timing_ptr_1: %08X", update_timing_ptr_1);
                ImGui::Text("update_timing_ptr_2: %08X", update_timing_ptr_2);
                ImGui::Text("update_timing_ptr_3: %08X", update_timing_ptr_3);
                ImGui::Text("update_timing_ptr_4: %08X", update_timing_ptr_4);
                ImGui::Text("window_manager_ptr: %08X", window_manager_ptr);
                ImGui::Text("dispatch_table_id: %08X", dispatch_table_id);
                ImGui::Text("dispatch_table_id_found: %s", nt_user_send_input_dispatch_table_id_found ? "Yes" : "No");
            }
            if (ImGui::CollapsingHeader("Methods", ImGuiTreeNodeFlags_None))
            {
                ImGui::Text("hom_update_vars_code_start: %08X", hom_update_vars_code_start);
                ImGui::Text("parse_beatmap_code_start: %08X", parse_beatmap_code_start);
                ImGui::Text("beatmap_onload_code_start: %08X", beatmap_onload_code_start);
                ImGui::Text("current_scene_code_start: %08X", current_scene_code_start);
                ImGui::Text("selected_song_code_start: %08X", selected_song_code_start);
                ImGui::Text("audio_time_code_start: %08X", audio_time_code_start);
                ImGui::Text("osu_manager_code_start: %08X", osu_manager_code_start);
                ImGui::Text("binding_manager_code_start: %08X", binding_manager_code_start);
                ImGui::Text("selected_replay_code_start: %08X", selected_replay_code_start);
                ImGui::Text("osu_client_id_code_start: %08X", osu_client_id_code_start);
                ImGui::Text("osu_username_code_start: %08X", osu_username_code_start);
                ImGui::Text("window_manager_code_start: %08X", window_manager_code_start);
                ImGui::Text("score_multiplier_code_start: %08X", score_multiplier_code_start);
                ImGui::Text("discord_rich_presence_code_start: %08X", discord_rich_presence_code_start);
                ImGui::Text("check_flashlight_code_start: %08X", check_flashlight_code_start);
                ImGui::Text("update_flashlight_code_start: %08X", update_flashlight_code_start);
                ImGui::Text("update_timing_code_start: %08X", update_timing_code_start);
                ImGui::Text("set_playback_rate_code_start: %08X", set_playback_rate_code_start);
                ImGui::Text("check_timewarp_code_start: %08X", check_timewarp_code_start);
            }
            if (ImGui::CollapsingHeader("Offsets", ImGuiTreeNodeFlags_None))
            {
                ImGui::Text("hom_update_vars_hidden_loc: 0x%X", hom_update_vars_hidden_loc);
                ImGui::Text("approach_rate_offsets: 0x%X 0x%X 0x%X", approach_rate_offsets[0], approach_rate_offsets[1], approach_rate_offsets[2]);
                ImGui::Text("circle_size_offsets: 0x%X 0x%X 0x%X", circle_size_offsets[0], circle_size_offsets[1], circle_size_offsets[2]);
                ImGui::Text("overall_difficulty_offsets: 0x%X 0x%X", overall_difficulty_offsets[0], overall_difficulty_offsets[1]);
                ImGui::Text("beatmap_onload_offset: 0x%X", beatmap_onload_offset);
                ImGui::Text("current_scene_offset: 0x%X", current_scene_offset);
                ImGui::Text("selected_replay_offset: 0x%X", selected_replay_offset);
                ImGui::Text("window_manager_offset: 0x%X", window_manager_offset);
                ImGui::Text("selected_song_offset: 0x%X", selected_song_offset);
                ImGui::Text("audio_time_offset: 0x%X", audio_time_offset);
                ImGui::Text("osu_manager_offset: 0x%X", osu_manager_offset);
                ImGui::Text("binding_manager_offset: 0x%X", binding_manager_offset);
                ImGui::Text("client_id_offset: 0x%X", client_id_offset);
                ImGui::Text("username_offset: 0x%X", username_offset);
                ImGui::Text("check_timewarp_offset: 0x%X", check_timewarp_offset);
            }
            if (ImGui::CollapsingHeader("Hook Jumps", ImGuiTreeNodeFlags_None))
            {
                ImGui::Text("discord_rich_presence_jump_back: %08X", discord_rich_presence_jump_back);
                ImGui::Text("ar_hook_jump_back: %08X", ar_hook_jump_back);
                ImGui::Text("beatmap_onload_hook_jump_back: %08X", beatmap_onload_hook_jump_back);
                ImGui::Text("check_timewarp_hook_1_jump_back: %08X", check_timewarp_hook_1_jump_back);
                ImGui::Text("check_timewarp_hook_2_jump_back: %08X", check_timewarp_hook_2_jump_back);
                ImGui::Text("cs_hook_jump_back: %08X", cs_hook_jump_back);
                ImGui::Text("od_hook_jump_back: %08X", od_hook_jump_back);
                ImGui::Text("score_multiplier_hook_jump_back: %08X", score_multiplier_hook_jump_back);
                ImGui::Text("selected_replay_hook_jump_back: %08X", selected_replay_hook_jump_back);
                ImGui::Text("set_playback_rate_jump_back: %08X", set_playback_rate_jump_back);
            }
        }
        ImGui::End(); // tab_content
        ImGui::EndPopup();
    }

    ImGui::End(); // freedom
    ImGui::PopFont();
}

void destroy_ui()
{
    // ImGui_ImplOpenGL3_Shutdown();
    // ImGui_ImplWin32_Shutdown();
    // ImGui::DestroyContext();
    UnhookWindowsHookEx(oWndProc);
}

void parameter_slider(uintptr_t selected_song_ptr, Parameter *p)
{
    const char *slider_fmt;
    if (!p->found)
    {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleColor(ImGuiCol_Text, ITEM_DISABLED);
        slider_fmt = p->error_message;
    }
    else
    {
        slider_fmt = p->slider_fmt;
    }
    if (!p->lock)
    {
        if (p->found && selected_song_ptr)
        {
            uintptr_t param_ptr = 0;
            if (internal_memory_read(g_process, selected_song_ptr, &param_ptr))
            {
                param_ptr += p->offset;
                internal_memory_read(g_process, param_ptr, &p->value);
            }
        }
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleColor(ImGuiCol_Text, p->found ? ITEM_DISABLED : ITEM_UNAVAILABLE);
        ImGui::PushID(slider_fmt);
        ImGui::SliderFloat("", &p->value, .0f, 11.0f, slider_fmt);
        ImGui::PopID();
        ImGui::PopStyleColor();
        ImGui::PopItemFlag();
    }
    else
    {
        ImGui::PushID(slider_fmt);
        ImGui::SliderFloat("", &p->value, .0f, 11.0f, slider_fmt);
        ImGui::PopID();
        if (ImGui::IsItemDeactivatedAfterEdit())
            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
    }
    ImGui::SameLine();
    ImGui::PushID(p->offset);
    if (ImGui::Checkbox("", &p->lock))
    {
        p->lock ? p->enable() : p->disable();
        ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
    }
    ImGui::PopID();
    if (!p->found)
    {
        ImGui::PopStyleColor();
        ImGui::PopItemFlag();
    }
    ImGui::Dummy(ImVec2(0.0f, 5.0f));
}

void draw_debug_log()
{
    if (show_debug_log_window)
    {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth(), ImGui::GetWindowPos().y), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(640.f, 480.f), ImGuiCond_Once);
        ImGui::PushFont(font);
        ImGui::Begin("Debug Log", NULL);
        debug_log.draw();
        ImGui::End();
        ImGui::PopFont();
    }
}
