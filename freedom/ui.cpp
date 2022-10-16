#include "ui.h"

ImFont *font = 0;
char song_name_u8[256] = {'F', 'r', 'e', 'e', 'd', 'o', 'm', '\0'};

WNDPROC oWndProc;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

void init_ui()
{
    oWndProc = (WNDPROC)SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    set_imgui_ini_handler();
    io.IniFilename = get_imgui_ini_filename(g_module);

    ImGui::LoadIniSettingsFromDisk(io.IniFilename);

    ImFontConfig config;
    config.OversampleH = config.OversampleV = 1;
    config.PixelSnapH = true;

    for (int size = 34; size > 16; size -= 2)
    {
        config.SizePixels = size;
        ImFont *f = io.Fonts->AddFontFromMemoryCompressedBase85TTF(victor_mono_font_compressed_data_base85, size, &config);
        if (size == cfg_font_size)
            font = f;
    }

    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
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

        update_tab("Replay", MenuTab::Replay);

        update_tab("Other", MenuTab::Other);
        update_tab("About", MenuTab::About);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(540.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(0.0f, ImGui::GetWindowHeight()), ImGuiCond_Always);
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
            ImGui::PushItemWidth(24.0f);
            ImGui::InputText("Left Click",  left_click,  2, ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_AutoSelectAll);
            ImGui::InputText("Right Click", right_click, 2, ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_AutoSelectAll);
            ImGui::PopItemWidth();
            ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing());
            ImGui::Text("Singletap only!");
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
            ImGui::Text("Partial support for sliders!");
        }
        if (selected_tab == MenuTab::Replay)
        {
            static bool replay_hardrock = false;
            static bool replay_use_aim = true;
            static bool replay_use_keys = true;
            ImGui::Text("%s", current_replay.song_name_u8);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("Selected Replay");
            ImGui::Text("%s - %.2f%% - %ux - %s", current_replay.author, current_replay.accuracy, current_replay.combo, current_replay.mods);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("Player, Accuracy, Mods");
            ImGui::Dummy(ImVec2(.0f, 2.f));
            if (ImGui::Checkbox("Enable", &cfg_replay_enabled))
                cfg_replay_enabled ? enable_replay_hooks() : disable_replay_hooks();
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("Usage: Open Replay Preview in-game to Select a Replay");
            ImGui::SameLine(210.0f);
            if (!cfg_replay_enabled)
            {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleColor(ImGuiCol_Text, ITEM_DISABLED);
            }
            if (ImGui::Checkbox("Hardrock", &replay_hardrock))
                FR_INFO("replay_hardrock is not implemented");
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("Convert Replay to/from Hardrock");
            ImGui::Dummy(ImVec2(.0f, 2.f));
            if (ImGui::Checkbox("Replay Aim", &replay_use_aim))
                FR_INFO("replay_use_aim is not implemented");
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("Aim According to Replay Data");
            ImGui::SameLine(210.0f);
            if (ImGui::Checkbox("Replay Keys", &replay_use_keys))
                FR_INFO("replay_use_keys is not implemented");
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("Press Keys According to Replay Data");
            if (!cfg_replay_enabled)
            {
                ImGui::PopStyleColor();
                ImGui::PopItemFlag();
            }
            ImGui::Text("Hardrock, Replay Aim, Replay Keys checkboxes are not implemented yet!");
        }
        if (selected_tab == MenuTab::Other)
        {
            ImGui::Text("Other Settings");
            ImGui::Dummy(ImVec2(0.0f, 5.0f));
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
        }
        if (selected_tab == MenuTab::About)
        {
            ImGui::Text("Ciremun's Freedom " FR_VERSION);
            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            ImGui::Text("Special Thanks to Maple Syrup");
            ImGui::Text("@mrflashstudio");
            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            ImGui::Text("Discord: Ciremun#8516");
        }
        ImGui::End();
        ImGui::EndPopup();
    }

    ImGui::End();
    ImGui::PopFont();
}

void parameter_slider(uintptr_t selected_song_ptr, Parameter *p)
{
    if (!p->found)
    {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleColor(ImGuiCol_Text, ITEM_DISABLED);
        p->slider_fmt = p->error_message;
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
        ImGui::PushID(p->slider_fmt);
        ImGui::SliderFloat("", &p->value, .0f, 11.0f, p->slider_fmt);
        ImGui::PopID();
        ImGui::PopStyleColor();
        ImGui::PopItemFlag();
    }
    else
    {
        ImGui::PushID(p->slider_fmt);
        ImGui::SliderFloat("", &p->value, .0f, 11.0f, p->slider_fmt);
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
