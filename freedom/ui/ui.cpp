#include "ui/ui.h"

ImFont *font = 0;
char song_name_u8[256] = "Freedom " FR_VERSION " is Loading!";

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
        ImGui::ClosePopupsOverWindow(0, false);
    }

    if ((ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsPopupOpen((ImGuiID)0, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
         && ((message->message >= WM_MOUSEFIRST && message->message <= WM_MOUSELAST) || message->message == WM_CHAR))
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
    style.FrameRounding = 2.f;
    style.TabRounding = 2.f;
    style.ScrollbarRounding = 2.f;
    style.GrabRounding = 2.f;
    style.FramePadding = ImVec2(style.FramePadding.x, 4.f);
    style.WindowBorderSize = .0f;
    style.FrameBorderSize = .0f;
    style.PopupBorderSize = .0f;
    style.ChildBorderSize = .0f;
    style.GrabMinSize = 20.f;
    style.ScrollbarSize = cfg_font_size * .65f;
    style.ItemSpacing = ImVec2(style.ItemSpacing.x, 6.f);
    style.ItemInnerSpacing = ImVec2(6.f, style.ItemInnerSpacing.y);
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
    style.Colors[ImGuiCol_Tab] = PURPLE;
    style.Colors[ImGuiCol_TabHovered] = MAGENTA;
    style.Colors[ImGuiCol_TabActive] = MAGENTA;
}

inline void init_imgui_fonts()
{
    ImGuiIO &io = ImGui::GetIO();
    ImFontConfig config;
    config.OversampleH = config.OversampleV = 1;
    config.PixelSnapH = true;
    config.GlyphRanges = io.Fonts->GetGlyphRangesCyrillic();

    for (int size = 40; size >= 18; size -= 2)
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

#ifdef FR_DEBUG
    IMGUI_CHECKVERSION();
#endif // FR_DEBUG
    ImGuiContext* ctx = ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    ctx->SettingsHandlers.clear();

    set_imgui_ini_handler();
    io.IniFilename = get_imgui_ini_filename(g_module);
    if (io.IniFilename == 0)
        { FR_ERROR("Couldn't get config path"); }
    else
        ImGui::LoadIniSettingsFromDisk(io.IniFilename);

    init_imgui_fonts();
    init_imgui_styles();

    ImGui_ImplWin32_Init(g_hwnd);
    pDevice ? ImGui_ImplDX9_Init(pDevice) : ImGui_ImplOpenGL3_Init();
}

static void colored_if_null(const char *label, uintptr_t ptr, bool draw_label = true)
{
    uintptr_t found = ptr;
    if (!found)
        ImGui::PushStyleColor(ImGuiCol_Text, ITEM_UNAVAILABLE);

    char id_str[64] = {0};
    IM_ASSERT(strlen(label) < IM_ARRAYSIZE(id_str));
    ImFormatString(id_str, IM_ARRAYSIZE(id_str), "##%s", label);

    char ptr_str[32] = {0};
    ImFormatString(ptr_str, IM_ARRAYSIZE(ptr_str), "%08X", ptr);

    if (draw_label)
    {
        ImGui::Text(label);
        ImGui::SameLine(ImGui::GetFontSize() * 8.f);
    }
    ImGui::PushItemWidth(ImGui::GetFontSize() * 4.f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(BLACK_TRANSPARENT));
    ImGui::InputText(id_str, ptr_str, 32, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleColor();
    ImGui::PopItemWidth();

    if (!found)
        ImGui::PopStyleColor();
}

static inline bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
{
    ImGui::PushItemWidth(ImGui::GetFontSize() * 16.f);
    bool value_changed = ImGui::SliderFloat(label, v, v_min, v_max, format, flags);
    ImGui::PopItemWidth();
    return value_changed;
}

void update_ui()
{
    if (!cfg_mod_menu_visible)
        return;

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
                prev_song_str_ptr = song_str_ptr;
            }
        }
    }

    ImGui::PushFont(font);

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
    ImGui::Begin("Freedom", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("%s", song_name_u8);

    if (memory_scan_progress < .99f)
    {
        static char overlay_buf[32] = {0};
        ImFormatString(overlay_buf, IM_ARRAYSIZE(overlay_buf), "Memory Scan: %.0f%%", memory_scan_progress * 100 + 0.01f);
        ImGui::ProgressBar(memory_scan_progress, ImVec2(ImGui::GetContentRegionAvail().x, .0f), overlay_buf);
    }

    ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetWindowHeight()), ImGuiCond_Appearing);
    if (ImGui::BeginPopupContextItem("##settings"))
    {
        static MenuTab selected_tab = MenuTab::Difficulty;

        const auto update_tab = [](const char *tab_name, MenuTab tab_type, bool highlight = false)
        {
            bool is_selected = selected_tab == tab_type;
            if (!is_selected && highlight)
                ImGui::PushStyleColor(ImGuiCol_Text, SILVER);
            if (ImGui::Selectable(tab_name, is_selected, ImGuiSelectableFlags_DontClosePopups))
            {
                selected_tab = tab_type;
                ImGui::SetNextWindowFocus();
            }
            if (!is_selected && highlight)
                ImGui::PopStyleColor();
        };

        const auto inactive_tab = [](const char *tab_name)
        {
            ImGui::BeginDisabled();
            ImGui::PushStyleColor(ImGuiCol_Text, LOG_ERROR);
            ImGui::Selectable(tab_name, false, ImGuiSelectableFlags_DontClosePopups);
            ImGui::PopStyleColor();
            ImGui::EndDisabled();
        };

        update_tab("Difficulty", MenuTab::Difficulty, ar_parameter.lock || cs_parameter.lock || od_parameter.lock);

        beatmap_onload_offset ? update_tab("Relax",  MenuTab::Relax, cfg_relax_lock)  : inactive_tab("Relax");
        beatmap_onload_offset ? update_tab("Aimbot", MenuTab::Aimbot, cfg_aimbot_lock) : inactive_tab("Aimbot");
        set_playback_rate_code_start ? update_tab("Timewarp", MenuTab::Timewarp, cfg_timewarp_enabled) : inactive_tab("Timewarp");
        selected_replay_offset ? update_tab("Replay", MenuTab::Replay, cfg_replay_enabled) : inactive_tab("Replay");

        update_tab("Mods", MenuTab::Mods, cfg_flashlight_enabled || cfg_hidden_remover_enabled || cfg_score_multiplier_enabled);
        update_tab("Misc", MenuTab::Misc, cfg_drpc_enabled);
        update_tab("About", MenuTab::About);

        if (ImGui::Selectable("Debug", false, ImGuiSelectableFlags_DontClosePopups))
        {
            cfg_show_debug_log = true;
            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(ImGui::GetFontSize() * 18.f, ImGui::GetWindowHeight()));
        ImGui::SetNextWindowSize(ImVec2(.0f, .0f), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth(), ImGui::GetWindowPos().y), ImGuiCond_Always);
        ImGui::Begin("##tab_content", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
        ImGui::PopStyleVar();
        if (selected_tab == MenuTab::Difficulty)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetFontSize() * .25f, ImGui::GetFontSize() * .25f));
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
            bool relax_checks_od = cfg_relax_checks_od;
            if (!relax_checks_od)
                ImGui::BeginDisabled();
            if (ImGui::Checkbox("Jumping Unstable Rate Window", &cfg_jumping_window))
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            if (!relax_checks_od)
                ImGui::EndDisabled();
        }
        if (selected_tab == MenuTab::Aimbot)
        {
            if (ImGui::Checkbox("Enable", &cfg_aimbot_lock))
            {
                cfg_aimbot_lock ? enable_notify_hooks() : disable_notify_hooks();
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            SliderFloat("##fraction_modifier", &cfg_fraction_modifier, .01f, 5.f, "Cursor Delay: %.2f");
            if (ImGui::IsItemDeactivatedAfterEdit())
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            ImGui::Dummy(ImVec2(.0f, .5f));
            ImGui::PushItemWidth(ImGui::GetFontSize() * 16.f);
            ImGui::SliderInt("##spins_per_minute", &cfg_spins_per_minute, 0, 600, "Spins Per Minute: %d");
            ImGui::PopItemWidth();
            if (ImGui::IsItemDeactivatedAfterEdit())
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
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
            ImGui::PushItemWidth(ImGui::GetFontSize() * 16.f);
            if (ImGui::SliderScalar("##timewarp_scale", ImGuiDataType_Double, &timewarp_playback_rate, &p_min, &p_max, "Timewarp Scale: %.2lf"))
                cfg_timewarp_playback_rate = timewarp_playback_rate * 100.0;
            ImGui::PopItemWidth();
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
            ImGui::SameLine(ImGui::GetFontSize() * 8.f);
            if (!cfg_replay_enabled)
                ImGui::BeginDisabled();
            if (ImGui::Checkbox("Hardrock", &cfg_replay_hardrock))         current_replay.toggle_hardrock();
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("Convert Replay to/from Hardrock");
            ImGui::Dummy(ImVec2(.0f, 2.f));
            if (ImGui::Checkbox("Replay Aim", &cfg_replay_aim))            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("Aim According to Replay Data");
            ImGui::SameLine(ImGui::GetFontSize() * 8.f);
            if (ImGui::Checkbox("Replay Keys", &cfg_replay_keys))          ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("Press Keys According to Replay Data");
            if (!cfg_replay_enabled)
                ImGui::EndDisabled();
        }
        if (selected_tab == MenuTab::Mods)
        {
            uintptr_t unmod_flashlight_found = update_flashlight_code_start;
            if (!unmod_flashlight_found)
                ImGui::BeginDisabled();
            if (ImGui::Checkbox("Unmod Flashlight", &cfg_flashlight_enabled))
            {
                cfg_flashlight_enabled ? enable_flashlight_hooks() : disable_flashlight_hooks();
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            if (!unmod_flashlight_found)
                ImGui::EndDisabled();
            ImGui::Dummy(ImVec2(.0f, 5.f));
            uintptr_t unmod_hidden_found = hom_update_vars_hidden_loc;
            if (!unmod_hidden_found)
                ImGui::BeginDisabled();
            if (ImGui::Checkbox("Unmod Hidden", &cfg_hidden_remover_enabled))
            {
                cfg_hidden_remover_enabled ? enable_hidden_remover_hooks() : disable_hidden_remover_hooks();
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, LOG_ERROR);
            ImGui::Text("(Detected!)");
            ImGui::PopStyleColor();
            if (!unmod_hidden_found)
                ImGui::EndDisabled();
            ImGui::Dummy(ImVec2(.0f, 5.f));
            uintptr_t score_multiplier_found = score_multiplier_offset;
            if (!score_multiplier_found)
                ImGui::BeginDisabled();
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
            SliderFloat("##score_multiplier", &cfg_score_multiplier_value, .0f, 100.f, "Score Multiplier: %.0f");
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("Hold Ctrl To Set a Custom Value");
            if (ImGui::IsItemDeactivatedAfterEdit())
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            if (!cfg_score_multiplier_enabled)
            {
                ImGui::PopStyleColor();
                ImGui::PopItemFlag();
            }
            if (!score_multiplier_found)
                ImGui::EndDisabled();
        }
        if (selected_tab == MenuTab::Misc)
        {
            ImGui::Text("Discord RPC Settings");
            ImGui::Dummy(ImVec2(.0f, 5.f));

            if (ImGui::Checkbox("Enable", &cfg_drpc_enabled))
            {
                cfg_drpc_enabled ? enable_drpc_hooks() : disable_drpc_hooks();
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }

            ImGui::Dummy(ImVec2(.0f, 5.f));

            if (!cfg_drpc_enabled)
            {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleColor(ImGuiCol_Text, ITEM_DISABLED);
            }
            ImGui::PushItemWidth(ImGui::GetFontSize() * 16.f);
            if (ImGui::InputTextEx("##rpc_state", "State", cfg_drpc_state, 512, ImVec2(0, 0), ImGuiInputTextFlags_None))
            {
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
                set_discord_rpc_str(drpc_state_wchar, cfg_drpc_state, &drpc_state_string_ptr, &drpc_state_string_gc_handle);
            }
            if (ImGui::InputTextEx("##rpc_large_text", "Large Text", cfg_drpc_large_text, 512, ImVec2(0, 0), ImGuiInputTextFlags_None))
            {
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
                set_discord_rpc_str(drpc_large_text_wchar, cfg_drpc_large_text, &drpc_large_text_string_ptr, &drpc_large_text_string_gc_handle);
            }
            if (ImGui::InputTextEx("##rpc_small_text", "Small Text", cfg_drpc_small_text, 512, ImVec2(0, 0), ImGuiInputTextFlags_None))
            {
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
                set_discord_rpc_str(drpc_small_text_wchar, cfg_drpc_small_text, &drpc_small_text_string_ptr, &drpc_small_text_string_gc_handle);
            }
            if (!cfg_drpc_enabled)
            {
                ImGui::PopStyleColor();
                ImGui::PopItemFlag();
            }

            ImGui::Dummy(ImVec2(.0f, 10.f));

            static char preview_font_size[16] = {0};
            stbsp_snprintf(preview_font_size, 16, "Font Size: %dpx", (int)ImGui::GetFontSize());
            if (ImGui::BeginCombo("##font_size", preview_font_size, ImGuiComboFlags_HeightLargest))
            {
                const ImGuiIO& io = ImGui::GetIO();
                for (const auto &f : io.Fonts->Fonts)
                {
                    char font_sz[8] = {0};
                    stbsp_snprintf(font_sz, 4, "%d", (int)f->ConfigData->SizePixels);
                    const bool is_selected = f == font;
                    if (ImGui::Selectable(font_sz, is_selected))
                    {
                        font = f;
                        cfg_font_size = (int)f->ConfigData->SizePixels;
                        ImGui::GetStyle().ScrollbarSize = cfg_font_size * .65f;
                        ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
                    }
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            ImGui::Dummy(ImVec2(.0f, 10.f));
            if (ImGui::Button("Rescan Memory"))
            {
                destroy_hooks_except_swap();
                CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)init_hooks, 0, 0 ,0));
            }
            ImGui::SameLine(.0f, ImGui::GetFontSize());
            if (ImGui::Button("Unload DLL"))
                unload_dll();
            ImGui::Dummy(ImVec2(.0f, 5.f));
        }
        if (selected_tab == MenuTab::About)
        {
            ImGui::Text("Ciremun's Freedom " FR_VERSION);
            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            ImGui::Text("Special Thanks to Maple Syrup");
            ImGui::Text("@mrflashstudio");
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
        ImGui::BeginDisabled();
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
        ImGui::PushID(slider_fmt);
        ImGui::BeginDisabled();
        SliderFloat("", &p->value, .0f, 11.0f, slider_fmt);
        ImGui::EndDisabled();
        ImGui::PopID();
    }
    else
    {
        ImGui::PushID(slider_fmt);
        if (SliderFloat("", &p->value, .0f, 11.0f, slider_fmt))
            p->apply_mods();
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
        ImGui::EndDisabled();
    ImGui::Dummy(ImVec2(0.0f, 5.0f));
}

void draw_debug_log()
{
    if (cfg_show_debug_log)
    {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth(), ImGui::GetWindowPos().y), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(640.f, 480.f), ImGuiCond_Once);
        ImGui::PushFont(font);
        ImGui::Begin("Debug", &cfg_show_debug_log, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16.f, ImGui::GetStyle().FramePadding.y));
        if (ImGui::BeginTabBar("##debug_tabs", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_NoTabListScrollingButtons))
        {
            if (ImGui::BeginTabItem("Log"))
            {
                ImGui::PopStyleVar(); // FramePadding
                debug_log.draw();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Game"))
            {
                ImGui::PopStyleVar();
                ImGui::BeginChild("##debug_game", ImVec2(.0f, -30.f));
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
                char selected_mods[64] = "Unknown";
                ImGui::Text("Selected Mods: %s", selected_mods_ptr ? mods_to_string(*selected_mods_ptr, selected_mods) : "Unknown");
                ImGui::Text("Replay Mode: %s", is_replay_mode(osu_manager_ptr) ? "Yes" : "No");
                ImGui::Dummy(ImVec2(.0f, 5.f));
                ImGui::Text("All Methods Found: %s", all_code_starts_found() ? "Yes" : "No");
                ImGui::Text("Prepared Methods: %d", prepared_methods_count);
                ImGui::TextWrapped("Config Path: %s", ImGui::GetIO().IniFilename);
                ImGui::EndChild(); // debug_game
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Scan"))
            {
                ImGui::PopStyleVar();
                ImGui::BeginChild("##debug_scan", ImVec2(.0f, -30.f));
                if (ImGui::CollapsingHeader("Account Info", ImGuiTreeNodeFlags_None))
                {
                    ImGui::Text("Client ID: \n%s", osu_client_id);
                    ImGui::Text("Username: %s", osu_username);
                }
                if (ImGui::CollapsingHeader("Playfield", ImGuiTreeNodeFlags_None))
                {
                    ImGui::Text("Window Size: %f %f", window_size.x, window_size.y);
                    ImGui::Text("Playfield Size: %f %f", playfield_size.x, playfield_size.y);
                    ImGui::Text("Playfield Position: %f %f", playfield_position.x, playfield_position.y);
                }
                if (ImGui::CollapsingHeader("Pointers", ImGuiTreeNodeFlags_None))
                {
                    colored_if_null("Audio Time", audio_time_ptr);
                    colored_if_null("Binding Manager", binding_manager_ptr);
                    colored_if_null("Osu Manager", osu_manager_ptr);
                    colored_if_null("Selected Replay", selected_replay_ptr);
                    colored_if_null("Selected Beatmap", selected_song_ptr);
                    colored_if_null("Selected Mods", (uintptr_t)selected_mods_ptr);
                    // colored_if_null("Update Timing 1", update_timing_ptr_1);
                    // colored_if_null("Update Timing 2", update_timing_ptr_2);
                    // colored_if_null("Update Timing 3", update_timing_ptr_3);
                    // colored_if_null("Update Timing 4", update_timing_ptr_4);
                    colored_if_null("Window Manager", window_manager_ptr);
                    colored_if_null("Dispatch Table ID", dispatch_table_id);
                    ImGui::Text("Dispatch Table ID Found: %s", nt_user_send_input_dispatch_table_id_found ? "Yes" : "No");
                }
                if (ImGui::CollapsingHeader("Methods", ImGuiTreeNodeFlags_None))
                {
                    colored_if_null("Parse Beatmap", parse_beatmap_code_start);
                    colored_if_null("Beatmap Onload", beatmap_onload_code_start);
                    colored_if_null("Current Scene", current_scene_code_start);
                    colored_if_null("Selected Beatmap", selected_song_code_start);
                    colored_if_null("Audio Time", audio_time_code_start);
                    colored_if_null("Osu Manager", osu_manager_code_start);
                    colored_if_null("Binding Manager", binding_manager_code_start);
                    colored_if_null("Selected Replay", selected_replay_code_start);
                    colored_if_null("Osu Client ID", osu_client_id_code_start);
                    colored_if_null("Osu Username", osu_username_code_start);
                    colored_if_null("Window Manager", window_manager_code_start);
                    colored_if_null("Score Multiplier", score_multiplier_code_start);
                    colored_if_null("Discord RPC", drpc_code_start);
                    colored_if_null("Check Flashlight", check_flashlight_code_start);
                    colored_if_null("Update Flashlight", update_flashlight_code_start);
                    colored_if_null("Update Timing", update_timing_code_start);
                    colored_if_null("Set Playback Rate", set_playback_rate_code_start);
                    colored_if_null("Check Timewarp", check_timewarp_code_start);
                    colored_if_null("Selected Mods", selected_mods_code_start);
                    colored_if_null("Update Mods", update_mods_code_start);
                }
                if (ImGui::CollapsingHeader("Offsets", ImGuiTreeNodeFlags_None))
                {
                    colored_if_null("Update Variables", hom_update_vars_hidden_loc);
                    colored_if_null("Beatmap Onload", beatmap_onload_offset);
                    colored_if_null("Current Scene", current_scene_offset);
                    colored_if_null("Selected Replay", selected_replay_offset);
                    colored_if_null("Window Manager", window_manager_offset);
                    colored_if_null("Selected Beatmap", selected_song_offset);
                    colored_if_null("Audio Time", audio_time_offset);
                    colored_if_null("Osu Manager", osu_manager_offset);
                    colored_if_null("Binding Manager", binding_manager_offset);
                    colored_if_null("Client ID", client_id_offset);
                    colored_if_null("Username", username_offset);
                    colored_if_null("Check Timewarp", check_timewarp_offset);
                    colored_if_null("Update Mods", update_mods_offset);
                    colored_if_null("Score Multiplier", score_multiplier_offset);
                    ImGui::Text("AR: %08X %08X %08X", approach_rate_offsets[0], approach_rate_offsets[1], approach_rate_offsets[2]);
                    ImGui::Text("CS: %08X %08X %08X", circle_size_offsets[0], circle_size_offsets[1], circle_size_offsets[2]);
                    ImGui::Text("OD: %08X %08X", overall_difficulty_offsets[0], overall_difficulty_offsets[1]);
                }
                if (ImGui::CollapsingHeader("Hooks", ImGuiTreeNodeFlags_None))
                {
                    const auto hook_info = [](const char *label, BYTE *hook_src, bool hook_enabled){
                        ImGui::Text(label);
                        ImGui::SameLine(.0f, 1.f);
                        ImGui::Text(": ");
                        ImGui::SameLine(ImGui::GetFontSize() * 8.f);
                        if (!hook_enabled)
                            ImGui::PushStyleColor(ImGuiCol_Text, ITEM_UNAVAILABLE);
                        ImGui::Text("%d", hook_enabled);
                        if (!hook_enabled)
                            ImGui::PopStyleColor();
                        ImGui::SameLine(ImGui::GetFontSize() * 9.f);
                        colored_if_null(label, (uintptr_t)hook_src, false);
                    };
                    hook_info("Scene", SceneHook.src, SceneHook.enabled);
                    hook_info("Update Mods", UpdateModsHook.src, UpdateModsHook.enabled);
                    hook_info("Swap Buffers", SwapBuffersHook.src, SwapBuffersHook.enabled);
                    hook_info("Beatmap On Load", BeatmapOnLoadHook.src, BeatmapOnLoadHook.enabled);
                    hook_info("AR 1", ApproachRateHook1.src, ApproachRateHook1.enabled);
                    hook_info("AR 2", ApproachRateHook2.src, ApproachRateHook2.enabled);
                    hook_info("AR 3", ApproachRateHook3.src, ApproachRateHook3.enabled);
                    hook_info("CS 1", CircleSizeHook1.src, CircleSizeHook1.enabled);
                    hook_info("CS 2", CircleSizeHook2.src, CircleSizeHook2.enabled);
                    hook_info("CS 3", CircleSizeHook3.src, CircleSizeHook3.enabled);
                    hook_info("OD 1", OverallDifficultyHook1.src, OverallDifficultyHook1.enabled);
                    hook_info("OD 2", OverallDifficultyHook2.src, OverallDifficultyHook2.enabled);
                    hook_info("Discord RPC", DiscordRichPresenceHook.src, DiscordRichPresenceHook.enabled);
                    hook_info("Hidden", HiddenHook.src, HiddenHook.enabled);
                    hook_info("Selected Replay", SelectedReplayHook.src, SelectedReplayHook.enabled);
                    hook_info("Score Multiplier", ScoreMultiplierHook.src, ScoreMultiplierHook.enabled);
                    hook_info("Set Playback Rate", SetPlaybackRateHook.src, SetPlaybackRateHook.enabled);
                    hook_info("Check Timewarp 1", CheckTimewarpHook1.src, CheckTimewarpHook1.enabled);
                    hook_info("Check Timewarp 2", CheckTimewarpHook2.src, CheckTimewarpHook2.enabled);
                }
                if (ImGui::CollapsingHeader("Hook Jumps", ImGuiTreeNodeFlags_None))
                {
                    colored_if_null("AR", ar_hook_jump_back);
                    colored_if_null("CS", cs_hook_jump_back);
                    colored_if_null("OD", od_hook_jump_back);
                    colored_if_null("Discord RPC", drpc_jump_back);
                    colored_if_null("Beatmap Onload", beatmap_onload_hook_jump_back);
                    colored_if_null("Check Timewarp 1", check_timewarp_hook_1_jump_back);
                    colored_if_null("Check Timewarp 2", check_timewarp_hook_2_jump_back);
                    colored_if_null("Score Multiplier", score_multiplier_hook_jump_back);
                    colored_if_null("Selected Replay", selected_replay_hook_jump_back);
                    colored_if_null("Set Playback Rate", set_playback_rate_jump_back);
                }
                ImGui::EndChild(); // debug_scan
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Beatmap"))
            {
                ImGui::PopStyleVar();
                ImGui::BeginChild("##debug_beatmap", ImVec2(.0f, -30.f));
                ImGui::Text("Current Beatmap");
                ImGui::Text("Hit Objects Count: %zu", current_beatmap.hit_objects.size());
                ImGui::Text("Hit Object Index: %u", current_beatmap.hit_object_idx);
                ImGui::Text("Hit Object Radius: %f", current_beatmap.hit_object_radius);
                ImGui::Text("Scaled Hit Object Radius: %f", current_beatmap.scaled_hit_object_radius);
                ImGui::Text("Ready: %s", current_beatmap.ready ? "Yes" : "No");
                char current_beatmap_mods[64] = "Unknown";
                ImGui::Text("Mods: %s", mods_to_string(current_beatmap.mods, current_beatmap_mods));
                if (current_beatmap.hit_objects.size() > 0)
                {
                    ImGui::Dummy(ImVec2(.0f, 5.f));
                    ImGui::Text("Current Circle:");
                    Circle& c = current_beatmap.current_circle();
                    ImGui::Text("Time: %d %d", c.start_time, c.end_time);
                    ImGui::Text("Clicked: %s", c.clicked ? "Yes" : "No");
                    ImGui::Text("Type: %d", c.type);
                    ImGui::Text("Position: %.2f %.2f", c.position.x, c.position.y);
                }
                ImGui::EndChild(); // debug_beatmap
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        else
            ImGui::PopStyleVar(); // FramePadding
        ImGui::End(); // Debug
        ImGui::PopFont();
        if (!cfg_show_debug_log)
            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
    }
}
