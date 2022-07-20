#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "CorGuids.lib")

#include "stdafx.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include "imgui_internal.h"
#include "stb_sprintf.h"

#include "config.h"
#include "detours.h"
#include "hook.h"
#include "offsets.h"
#include "utility.h"

#define ITEM_DISABLED ImVec4(0.50f, 0.50f, 0.50f, 1.00f)
#define ITEM_UNAVAILABLE ImVec4(1.0f, 0.0f, 0.0f, 1.00f)

bool beatmap_loaded = false;
bool start_parse_beatmap = false;
Scene current_scene = Scene::MAIN_MENU;
uintptr_t osu_auth_base = 0;
BeatmapData current_beatmap;

HWND g_hwnd = NULL;
HANDLE g_process = NULL;
HMODULE g_module = NULL;

void parameter_slider(uintptr_t current_song_ptr, Parameter *p)
{
    if (!p->found)
    {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleColor(ImGuiCol_Text, ITEM_DISABLED);
        p->slider_fmt = p->error_message;
    }
    if (!p->lock)
    {
        if (p->found && current_song_ptr)
        {
            uintptr_t param_ptr = 0;
            if (internal_memory_read(g_process, current_song_ptr, &param_ptr))
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
            ImFont *f = io.Fonts->AddFontDefault(&config);
            if (size == cfg_font_size)
                font = f;
        }

        try_find_hook_offsets();
        init_hooks();

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

    if (GetAsyncKeyState(VK_F11) & 1)
    {
        cfg_mod_menu_visible = !cfg_mod_menu_visible;
        ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
    }

    if (!cfg_mod_menu_visible)
        return wglSwapBuffersGateway(hDc);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiIO &io = ImGui::GetIO();
    ImGui::PushFont(font);

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
    ImGui::Begin("Freedom", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

    osu_auth_base = GetModuleBaseAddress(L"osu!auth.dll");
    static uintptr_t current_song_ptr = internal_multi_level_pointer_dereference(g_process, osu_auth_base + selected_song_ptr_base_offset, selected_song_ptr_offsets);
    static char song_name_u8[128] = {'F', 'r', 'e', 'e', 'd', 'o', 'm', '\0'};
    if (current_song_ptr)
    {
        uintptr_t song_str_ptr = 0;
        if (internal_memory_read(g_process, current_song_ptr, &song_str_ptr))
        {
            if (start_parse_beatmap)
            {
                bool success = false;
                uintptr_t folder_name_ptr = 0;
                if (internal_memory_read(g_process, song_str_ptr + 0x78, &folder_name_ptr))
                {
                    const wchar_t *folder_name = (const wchar_t *)(folder_name_ptr + 0x8);
                    uint32_t folder_name_length = *(uint32_t *)(folder_name_ptr + 0x4);
                    uintptr_t diff_name_ptr = 0;
                    if (internal_memory_read(g_process, song_str_ptr + 0x94, &diff_name_ptr))
                    {
                        const wchar_t *diff_name = (const wchar_t *)(diff_name_ptr + 0x8);
                        uint32_t diff_name_length = *(uint32_t *)(diff_name_ptr + 0x4);
                        static TCHAR osu_path[MAX_PATH * 2];
                        DWORD osu_path_length = GetModuleFileNameEx(g_process, NULL, osu_path, MAX_PATH);
                        if (osu_path_length == 0)
                            FR_ERROR_FMT("GetModuleFileNameEx failed: %d", GetLastError());
                        DWORD backslash_index = osu_path_length - 1;
                        while (backslash_index)
                            if (osu_path[--backslash_index] == '\\')
                                break;
                        osu_path[backslash_index] = '\0';
                        _snwprintf(osu_path, osu_path_length + 7 + folder_name_length + 1 + diff_name_length, L"%s\\Songs\\%s\\%s", osu_path, folder_name, diff_name);
                        FR_INFO_FMT("parsing beatmap: %S", osu_path);
                        success = parse_beatmap(osu_path, current_beatmap);
                    }
                }
                if (!success)
                    FR_ERROR("couldn't parse beatmap");
                start_parse_beatmap = false;
            }
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
    else
    {
        current_song_ptr = internal_multi_level_pointer_dereference(g_process, osu_auth_base + selected_song_ptr_base_offset, selected_song_ptr_offsets);
    }

    static uintptr_t audio_time_ptr = internal_multi_level_pointer_dereference(g_process, osu_auth_base + audio_time_ptr_base_offset, audio_time_ptr_offsets);

    if (beatmap_loaded && current_scene == Scene::GAMIN && current_beatmap.parsed_successfully)
    {
        double current_time = ImGui::GetTime();
        static double delayed_keyup = 0.0;
        static bool last = false;
        int32_t audio_time = *(int32_t *)audio_time_ptr;
        Circle circle = current_beatmap.current_circle();
        if (circle.type == CircleType::POINT && audio_time >= circle.hit_circle.time)
        {
            FR_INFO_FMT("hit %d!, %d >= %d", current_beatmap.hit_object_idx, audio_time, circle.hit_circle.time);

            INPUT inputs[1];
            inputs[0].type = INPUT_KEYBOARD;
            inputs[0].ki.wVk = 0;
            inputs[0].ki.wScan = 0;
            inputs[0].ki.time = 0;
            inputs[0].ki.dwExtraInfo = 0;
            inputs[0].ki.dwFlags = 0;
            inputs[0].ki.wVk = 'S';
            SendInput(1, inputs, sizeof(INPUT));

            delayed_keyup = ImGui::GetTime();

            current_beatmap.hit_object_idx++;
            if (current_beatmap.hit_object_idx >= current_beatmap.hit_objects.size())
                last = true;
        }
        if (delayed_keyup && ((current_time - delayed_keyup) > 0.0005))
        {
            delayed_keyup = 0.0;

            INPUT inputs[1];
            inputs[0].type = INPUT_KEYBOARD;
            inputs[0].ki.wVk = 0;
            inputs[0].ki.wScan = 0;
            inputs[0].ki.time = 0;
            inputs[0].ki.dwExtraInfo = 0;
            inputs[0].ki.dwFlags = KEYEVENTF_KEYUP;
            inputs[0].ki.wVk = 'S';
            SendInput(1, inputs, sizeof(INPUT));
            if (last)
            {
                beatmap_loaded = false;
                last = false;
            }
        }
    }

    ImGui::Text("%s", song_name_u8);

    ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetWindowHeight()), ImGuiCond_Appearing);
    if (ImGui::BeginPopupContextItem("##settings"))
    {
        parameter_slider(current_song_ptr, &ar_parameter);
        parameter_slider(current_song_ptr, &cs_parameter);
        parameter_slider(current_song_ptr, &od_parameter);

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
        ImGui::EndPopup();
    }

    ImGui::End();
    ImGui::PopFont();

    io.MouseDrawCursor = io.WantCaptureMouse;

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
