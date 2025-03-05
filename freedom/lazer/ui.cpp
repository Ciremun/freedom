#include "ui/ui.h"

ImFont *font = 0;
char song_name_u8[256] = "Freedom " FR_VERSION;

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
    style.Colors[ImGuiCol_TabSelected] = MAGENTA;
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
        config.SizePixels = (float)size;
        ImFont *f = io.Fonts->AddFontFromMemoryCompressedBase85TTF(victor_mono_font_compressed_data_base85, (float)size, &config);
        if (size == cfg_font_size)
            font = f;
    }
}

void init_ui(ID3D11Device* p_device, ID3D11DeviceContext* p_context)
{
#ifdef FR_DEBUG
    IMGUI_CHECKVERSION();
#endif // FR_DEBUG
    ImGuiContext* ctx = ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    ctx->SettingsHandlers.clear();

    set_imgui_ini_handler();
    io.IniFilename = get_imgui_ini_filename(g_module);
    if (io.IniFilename == 0)
        FR_ERROR("Couldn't get config path");
    else
        ImGui::LoadIniSettingsFromDisk(io.IniFilename);

    init_imgui_fonts();
    init_imgui_styles();

    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX11_Init(p_device, p_context);
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

static void SliderDifficultySetting(DifficultySetting *p)
{
    IM_ASSERT(p != 0);
    ImGui::PushID(p->fmt);
    if (!p->enabled)
    {
        ImGui::BeginDisabled();
        SliderFloat("", &p->value, .0f, 11.0f, p->fmt);
        ImGui::EndDisabled();
    }
    else
    {
        SliderFloat("", &p->value, .0f, 11.0f, p->fmt);
        if (ImGui::IsItemDeactivatedAfterEdit())
            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
    }
    ImGui::PopID(); // PushID(p->fmt)
    ImGui::SameLine();
    ImGui::PushID(p->label);
    if (ImGui::Checkbox("", &p->enabled))
        ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
    ImGui::PopID(); // PushID(p->label)
    ImGui::Dummy(ImVec2(0.0f, 5.0f));
}

void update_ui()
{
    if (!cfg_mod_menu_visible)
        return;

    ImGui::PushFont(font);

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
    ImGui::Begin("Freedom", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("%s", song_name_u8);

    ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetWindowHeight()), ImGuiCond_Appearing);
    if (ImGui::BeginPopupContextItem("##settings"))
    {
        static MenuTab selected_tab = MenuTab::Difficulty;

        const auto update_tab = [](const char *tab_name, MenuTab tab_type, bool highlight = false)
        {
            bool is_selected = selected_tab == tab_type;
            if (!is_selected && highlight)
                ImGui::PushStyleColor(ImGuiCol_Text, SILVER);
            if (ImGui::Selectable(tab_name, is_selected, ImGuiSelectableFlags_NoAutoClosePopups))
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
            ImGui::Selectable(tab_name, false, ImGuiSelectableFlags_NoAutoClosePopups);
            ImGui::PopStyleColor();
            ImGui::EndDisabled();
        };

        update_tab("Difficulty", MenuTab::Difficulty, ar_setting.enabled || cs_setting.enabled || od_setting.enabled);

        false ? update_tab("Relax",  MenuTab::Relax, cfg_relax_lock)  : inactive_tab("Relax");
        false ? update_tab("Aimbot", MenuTab::Aimbot, cfg_aimbot_lock) : inactive_tab("Aimbot");
        false ? update_tab("Timewarp", MenuTab::Timewarp, cfg_timewarp_enabled) : inactive_tab("Timewarp");
        false ? update_tab("Replay", MenuTab::Replay, cfg_replay_enabled) : inactive_tab("Replay");

        update_tab("Mods", MenuTab::Mods, cfg_flashlight_enabled || cfg_hidden_remover_enabled || cfg_score_multiplier_enabled);
        update_tab("Misc", MenuTab::Misc, cfg_drpc_enabled);
        update_tab("About", MenuTab::About);

        if (ImGui::Selectable("Debug", false, ImGuiSelectableFlags_NoAutoClosePopups))
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
            SliderDifficultySetting(&ar_setting);
            SliderDifficultySetting(&cs_setting);
            SliderDifficultySetting(&od_setting);
            ImGui::PopStyleVar();
        }
        if (selected_tab == MenuTab::Relax) {}
        if (selected_tab == MenuTab::Aimbot) {}
        if (selected_tab == MenuTab::Timewarp) {}
        if (selected_tab == MenuTab::Replay) {}
        if (selected_tab == MenuTab::Mods) {}
        if (selected_tab == MenuTab::Misc)
        {
            static char preview_font_size[16] = {0};
            stbsp_snprintf(preview_font_size, 16, "Font Size: %dpx", (int)ImGui::GetFontSize());
            ImGui::PushItemWidth(ImGui::GetFontSize() * 16.f);
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
        }
        if (selected_tab == MenuTab::About)
        {
            ImGui::Text("Ciremun's Freedom " FR_VERSION);
        }
        ImGui::End(); // tab_content
        ImGui::EndPopup();
    }
    ImGui::End(); // freedom
    ImGui::PopFont();
}

void destroy_ui()
{
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
                ImGui::EndTabItem(); // Log
            }
            if (ImGui::BeginTabItem("Game"))
            {
                ImGui::PopStyleVar();
                ImGui::BeginChild("##debug_game", ImVec2(.0f, -30.f));
                ImGui::TextWrapped("Config Path: %s", ImGui::GetIO().IniFilename);
                ImGui::EndChild(); // debug_game
                ImGui::EndTabItem(); // Game
            }
            ImGui::EndTabBar(); // ##debug_tabs
        }
        else
            ImGui::PopStyleVar(); // FramePadding
        ImGui::End(); // Debug
        ImGui::PopFont(); // font
        if (!cfg_show_debug_log)
            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
    }
}
