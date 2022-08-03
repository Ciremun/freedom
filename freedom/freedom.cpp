#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "CorGuids.lib")

#include "stdafx.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include "imgui_internal.h"
#include "stb_sprintf.h"

#include "aimbot.h"
#include "config.h"
#include "detours.h"
#include "hook.h"
#include "input.h"
#include "offsets.h"
#include "tabs.h"
#include "utility.h"
#include "window.h"
#include "font.h"

#define FR_VERSION "v0.5"
#define ITEM_DISABLED ImVec4(0.50f, 0.50f, 0.50f, 1.00f)
#define ITEM_UNAVAILABLE ImVec4(1.0f, 0.0f, 0.0f, 1.00f)

bool start_parse_beatmap = false;
bool target_first_circle = true;

uintptr_t osu_auth_base = 0;
char left_click[2] = { 'Z', '\0' };
char right_click[2] = { 'X', '\0' };

BeatmapData current_beatmap;
Scene current_scene = Scene::MAIN_MENU;

HWND g_hwnd = NULL;
HANDLE g_process = NULL;
HMODULE g_module = NULL;

WNDPROC oWndProc;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK find_osu_window(HWND hwnd, LPARAM lParam)
{
    DWORD lpdwProcessId;
    GetWindowThreadProcessId(hwnd, &lpdwProcessId);
    if (lpdwProcessId == lParam)
    {
        g_hwnd = hwnd;
        return FALSE;
    }
    return TRUE;
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

BOOL __stdcall freedom_update(HDC hDc)
{
    static ImFont *font = 0;
    static bool init = false;
    if (!init)
    {
        EnumWindows(find_osu_window, GetCurrentProcessId());

        if (!IsWindowVisible(g_hwnd))
            return wglSwapBuffersGateway(hDc);

        oWndProc = (WNDPROC)SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

#ifndef NDEBUG
        AllocConsole();
        FILE *f;
        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONOUT$", "w", stderr);
#endif // NDEBUG

        g_process = GetCurrentProcess();
        osu_auth_base = GetModuleBaseAddress(L"osu!auth.dll");

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

        try_find_hook_offsets();
        init_hooks();

        uintptr_t binding_manager_ptr = internal_multi_level_pointer_dereference(g_process, osu_auth_base + binding_manager_base_offset, binding_manager_ptr_offsets);
        if (binding_manager_ptr)
        {
            char sus_left_click = '\0';
            if (internal_memory_read(g_process, binding_manager_ptr, &sus_left_click))
            {
                char sus_right_click = '\0';
                if (internal_memory_read(g_process, binding_manager_ptr + 0x10, &sus_right_click))
                {
                    if (('A' <= sus_left_click && sus_left_click <= 'Z') && ('A' <= sus_right_click && sus_right_click <= 'Z'))
                    {
                        left_click[0] = sus_left_click;
                        right_click[0] = sus_right_click;
                    }
                }
            }
        }
        FR_INFO_FMT("left_click: %c", left_click[0]);
        FR_INFO_FMT("right_click: %c", right_click[0]);

        calc_playfield(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(g_hwnd);
        ImGui_ImplOpenGL3_Init();

        ImGuiStyle &style = ImGui::GetStyle();
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

#define BLACK ImVec4(0.05f, 0.05f, 0.05f, 1.0f)
#define WHITE ImVec4(1.0f, 1.0f, 1.0f, 1.0f)
#define BLACK_TRANSPARENT ImVec4(0.05f, 0.05f, 0.05f, 0.7f)
#define PURPLE ImVec4(0.28f, 0.05f, 0.66f, 1.0f)
#define MAGENTA ImVec4(0.34f, 0.04f, 0.68f, 1.0f)
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

        init = true;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    static char song_name_u8[128] = {'F', 'r', 'e', 'e', 'd', 'o', 'm', '\0'};
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
                        int bytes_written = WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)song_str, song_str_length, song_name_u8, 127, 0, 0);
                        song_name_u8[bytes_written] = '\0';
                    }
                }
            }
            prev_song_str_ptr = song_str_ptr;
        }
    }

    static uintptr_t osu_player_ptr = internal_multi_level_pointer_dereference(g_process, osu_auth_base + osu_player_ptr_base_offset, osu_player_ptr_offsets);

    if (start_parse_beatmap)
    {
        parse_beatmap(osu_player_ptr, current_beatmap);
        target_first_circle = true;
        start_parse_beatmap = false;
    }

    static double keydown_time = 0.0;
    static double keyup_delay = 0.0;
    static float fraction_modifier = 0.04f;
    static float fraction_of_the_distance = 0.0f;
    static Vector2 direction(0.0f, 0.0f);
    static Vector2 mouse_position(0.0f, 0.0f);
    if ((cfg_relax_lock || cfg_aimbot_lock) && current_scene == Scene::GAMIN && current_beatmap.ready)
    {
        double current_time = ImGui::GetTime();
        int32_t audio_time = *(int32_t *)audio_time_ptr;
        Circle circle = current_beatmap.current_circle();
        if (cfg_aimbot_lock)
        {
            if (fraction_of_the_distance)
            {
                if (fraction_of_the_distance > 1.0f)
                {
                    fraction_of_the_distance = 0.0f;
                }
                else
                {
                    Vector2 next_mouse_position = mouse_position + direction * fraction_of_the_distance;
                    move_mouse_to(next_mouse_position.x, next_mouse_position.y);
                    fraction_of_the_distance += fraction_modifier;
                }
            }
            if (target_first_circle)
            {
                direction = prepare_hitcircle_target(osu_player_ptr, circle, mouse_position);
                fraction_of_the_distance = fraction_modifier;
                target_first_circle = false;
            }
        }
        if (audio_time >= circle.start_time)
        {
            if (cfg_aimbot_lock && (current_beatmap.hit_object_idx + 1 < current_beatmap.hit_objects.size()) &&
                current_beatmap.hit_objects[current_beatmap.hit_object_idx + 1].type != HitObjectType::Spinner)
            {
                Circle circle_to_aim = current_beatmap.hit_objects[current_beatmap.hit_object_idx + 1];
                direction = prepare_hitcircle_target(osu_player_ptr, circle_to_aim, mouse_position);
                fraction_of_the_distance = fraction_modifier;
            }
            if (cfg_relax_lock)
            {
                send_keyboard_input(left_click[0], 0);
                FR_INFO_FMT("hit %d!, %d %d", current_beatmap.hit_object_idx, circle.start_time, circle.end_time);
                keyup_delay = circle.end_time ? circle.end_time - circle.start_time : 0.5;
                if (circle.type == HitObjectType::Slider || circle.type == HitObjectType::Spinner)
                {
                    if (current_beatmap.mods & Mods::DoubleTime)
                        keyup_delay /= 1.5;
                    else if (current_beatmap.mods & Mods::HalfTime)
                        keyup_delay /= 0.75;
                }
                keydown_time = ImGui::GetTime();
            }
            current_beatmap.hit_object_idx++;
            if (current_beatmap.hit_object_idx >= current_beatmap.hit_objects.size())
                current_beatmap.ready = false;
        }
    }
    if (cfg_relax_lock && keydown_time && ((ImGui::GetTime() - keydown_time) * 1000.0 > keyup_delay))
    {
        keydown_time = 0.0;
        send_keyboard_input(left_click[0], KEYEVENTF_KEYUP);
    }

    if (GetAsyncKeyState(VK_F11) & 1)
    {
        cfg_mod_menu_visible = !cfg_mod_menu_visible;
        ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
    }

    if (!cfg_mod_menu_visible)
        goto frame_end;

    ImGuiIO &io = ImGui::GetIO();
    ImGui::PushFont(font);

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
    ImGui::Begin("Freedom", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("%s", song_name_u8);

    ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetWindowHeight()), ImGuiCond_Appearing);
    if (ImGui::BeginPopupContextItem("##settings"))
    {
        static MenuTab selected_tab = MenuTab::Difficulty;

        if (ImGui::Selectable("Difficulty", selected_tab == MenuTab::Difficulty, ImGuiSelectableFlags_DontClosePopups))
        {
            selected_tab = MenuTab::Difficulty;
            ImGui::SetNextWindowFocus();
        }

        if (ImGui::Selectable("Relax", selected_tab == MenuTab::Relax, ImGuiSelectableFlags_DontClosePopups))
        {
            selected_tab = MenuTab::Relax;
            ImGui::SetNextWindowFocus();
        }

        if (ImGui::Selectable("Aimbot", selected_tab == MenuTab::Aimbot, ImGuiSelectableFlags_DontClosePopups))
        {
            selected_tab = MenuTab::Aimbot;
            ImGui::SetNextWindowFocus();
        }

        if (ImGui::Selectable("Other", selected_tab == MenuTab::Other, ImGuiSelectableFlags_DontClosePopups))
        {
            selected_tab = MenuTab::Other;
            ImGui::SetNextWindowFocus();
        }

        if (ImGui::Selectable("About", selected_tab == MenuTab::About, ImGuiSelectableFlags_DontClosePopups))
        {
            selected_tab = MenuTab::About;
            ImGui::SetNextWindowFocus();
        }

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
            ImGui::Text("Enable");
            ImGui::SameLine();
            if (ImGui::Checkbox("##relax_checkbox", &cfg_relax_lock))
            {
                cfg_relax_lock ? enable_notify_hooks() : disable_notify_hooks();
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::PushItemWidth(24.0f);
            ImGui::InputText("Left Click",  left_click,  2, ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_AutoSelectAll);
            ImGui::InputText("Right Click", right_click, 2, ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_AutoSelectAll);
            ImGui::PopItemWidth();
            ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing());
            ImGui::Text("Singletap only!");
        }
        if (selected_tab == MenuTab::Aimbot)
        {
            ImGui::Text("Enable");
            ImGui::SameLine();
            if (ImGui::Checkbox("##aimbot_checkbox", &cfg_aimbot_lock))
            {
                cfg_aimbot_lock ? enable_notify_hooks() : disable_notify_hooks();
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            if (ImGui::SliderFloat("##fraction_modifier", &fraction_modifier, 0.001f, 0.5f, "Cursor Speed: %.3f"))
            {
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
            }
            // ImGui::Dummy(ImVec2(0.0f, 5.0f));
            ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing());
            ImGui::Text("HitCircles only!");
        }
        if (selected_tab == MenuTab::Other)
        {
            ImGui::Text("Other Settings");
            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            ImGuiContext &g = *ImGui::GetCurrentContext();
            static char preview_font_size[16] = {0};
            stbsp_snprintf(preview_font_size, 16, "Font Size: %dpx", (int)g.Font->ConfigData->SizePixels);
            if (ImGui::BeginCombo("##font_size", preview_font_size, ImGuiComboFlags_HeightLargest))
            {
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

    io.MouseDrawCursor = io.WantCaptureMouse;

frame_end:

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return wglSwapBuffersGateway(hDc);
}

DWORD WINAPI freedom_main(HMODULE hModule)
{
    SwapBuffersHook = Hook("wglSwapBuffers", "opengl32.dll", (BYTE *)freedom_update, (BYTE *)&wglSwapBuffersGateway, 5);
    SwapBuffersHook.Enable();

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        g_module = hModule;
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)freedom_main,
                                 hModule, 0, nullptr));
    }
    break;
    default:
        break;
    }
    return TRUE;
}
