#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "CorGuids.lib")
// #pragma comment(lib, "Opengl32.lib")

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

// #include <gl/GL.h>

// int x = 100;
// int y = 100;
// int width = 10;
// int height = 10;
int hit_objects_ms[] = { 107, 380, 516, 789, 925, 1198, 1334, 1607, 1743, 2016, 2152, 2289, 2561, 2698, 2970, 3107, 3380, 3516, 3789, 3925, 4198, 4334, 4470, 4743, 4880, 5152, 5289, 5561, 5698, 5970, 6107, 6380, 6516, 6652, 6925, 7061, 7334, 7470, 7743, 7880, 8152, 8289, 8425, 8561, 8698, 8834, 8970, 18652, 20834, 22198, 22470, 22743, 22880, 23016, 23152, 23698, 25743, 26561, 26970, 28061, 28743, 30925, 31880, 32425, 33789, 33925, 34061, 34470, 34607, 35152, 40743, 40811, 40880, 40948, 41016, 41084, 41152, 41220, 41289, 41357, 41425, 41493, 41561, 41630, 41698, 41766, 41834, 41902, 41970, 42039, 42107, 42175, 42243, 42311, 42380, 42448, 42516, 42584, 42652, 42720, 42789, 42857, 44016, 44084, 44152, 44220, 44289, 44357, 44425, 44493, 44561, 44630, 44698, 44766, 44834, 44902, 44970, 45039, 45380, 45789, 45925, 46061, 46743, 46880, 48380, 48516, 48789, 48925, 48993, 49061, 49130, 49198, 49266, 49334, 49402, 50970, 51516, 58198, 65834, 65970, 66243, 66380, 66652, 66789, 67743, 67880, 68425, 70198, 70607, 75925, 76334, 76470, 76607, 77016, 77698, 77834, 77902, 77970, 78039, 78107, 78175, 78243, 78311, 78380, 78448, 78516, 78584, 78652, 78720, 78789, 78857, 79198, 79880, 80289, 80834, 80970, 82198, 82266, 82334, 82402, 82470, 82539, 82607, 82675, 82743, 82811, 82880, 82948, 83016, 83084, 83152, 83220, 83289, 83425, 83698, 83834, 84107, 84243, 85061, 85334, 86152, 86425, 86561, 86630, 86698, 86766, 86834, 86902, 86970, 87039, 87107, 87175, 87243, 87311, 87380, 87448, 87516, 87584, 88743, 88811, 88879, 88947, 89015, 89084, 89152, 89220, 89288, 89356, 89425, 89493, 89561, 89629, 89697, 89765, 89834, 89902, 89970, 90038, 90106, 90175, 90243, 90311, 90379, 90447, 90516, 90584, 90652, 90720, 90788, 90856, 90925, 90993, 91061, 91129, 91197, 91266, 91334, 91402, 91470, 91538, 91607, 91675, 91743, 91811, 91879, 91947, 92016, 92084, 92152, 92220, 92289, 92357, 92834, 92970, 93243, 93380, 93652, 93789, 94061, 94334, 94470, 94743, 94880, 95152, 95425, 95561, 95834, 95970, 96243, 96516, 96652, 96925, 97061, 97334, 97607, 97743, 98016, 98152, 98425, 98561, 98630, 98698, 98766, 98834, 98902, 98970, 99039, 99107, 99175, 99243, 99311, 99380, 99448, 99516, 99584, 99652, 99720, 99789, 99857, 99925, 99993, 100061, 100130, 100198, 100266, 100334, 100402, 100470, 100539, 100607, 100675, 100743, 100811, 100880, 100948, 101016, 101084, 101152, 101220, 101289, 101357, 101425, 101493, 101561, 101630, 101698, 110561, 110970, 111380, 111789, 112198, 112470, 112743, 113152, 113561, 113970, 114380, 114516, 114652, 115334, 115470, 116152, 116561, 117107, 117652, 118334, 118743, 119016, 119289, 119834, 120516, 120925, 121198, 121470, 122016, 122561, 122834, 122970, 123243, 123380, 123516, 132380, 132448, 132516, 132584, 132652, 132720, 132789, 132857, 132925, 132993, 133061, 133130, 133198, 133266, 133334, 133402, 133470, 133539, 133607, 133675, 133743, 133811, 133880, 133948, 134016, 134084, 134152, 134220, 134289, 134357, 134425, 134493, 135652 };
int hit_objects_ms_idx = 0;

#define ITEM_DISABLED ImVec4(0.50f, 0.50f, 0.50f, 1.00f)
#define ITEM_UNAVAILABLE ImVec4(1.0f, 0.0f, 0.0f, 1.00f)

HWND g_hwnd = NULL;
HANDLE g_process = NULL;
HMODULE g_module = NULL;

void send_input(const char *msg, int size)
{
    INPUT *inputs = (INPUT *)alloca(size * 2 * sizeof(INPUT));
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = 0;
    input.ki.wScan = 0;
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    input.ki.dwFlags = 0;

    for (int i = 0; i < size; i++)
    {
        input.ki.wVk = msg[i];
        inputs[i] = input;
    }
    for (int i = 0; i < size; i++)
    {
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        input.ki.wVk = msg[i];
        inputs[i + size] = input;
    }

    SendInput(size * 2, inputs, sizeof(INPUT));
}

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

    static double playback_start_time = 0.0;

    static uintptr_t osu_auth_base = GetModuleBaseAddress(L"osu!auth.dll");
    static uintptr_t current_song_ptr = internal_multi_level_pointer_dereference(g_process, osu_auth_base + selected_song_ptr_base_offset, selected_song_ptr_offsets);
    static char song_name_u8[128] = {'F', 'r', 'e', 'e', 'd', 'o', 'm', '\0'};
    if (current_song_ptr)
    {
        uintptr_t song_str_ptr = 0;
        if (internal_memory_read(g_process, current_song_ptr, &song_str_ptr))
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
        uintptr_t song_select_ui_ptr = 0;
        if (internal_memory_read(g_process, current_song_ptr + 0x18, &song_select_ui_ptr))
        {
            static uintptr_t prev_song_select_ui_ptr = 0;
            if (song_select_ui_ptr != prev_song_select_ui_ptr && song_select_ui_ptr == 0)
            {
                playback_start_time = ImGui::GetTime();
                hit_objects_ms_idx = 0;
                // printf("playback started!\n");
            }
            prev_song_select_ui_ptr = song_select_ui_ptr;
        }
    }
    else
    {
        current_song_ptr = internal_multi_level_pointer_dereference(g_process, osu_auth_base + selected_song_ptr_base_offset, selected_song_ptr_offsets);
    }

    // static float delay = 2.5f;
    // ImGui::SliderFloat("#delay", &delay, 0.0f, 6.0f, "%.2f");

    static double true_playback_start_time = 0.0;
    if (GetAsyncKeyState('D') & 1)
    {
        true_playback_start_time = ImGui::GetTime();
        hit_objects_ms_idx++;
        // printf("set true_playback_start_time\n");
    }

    if (playback_start_time && true_playback_start_time)
    {
        double current_time = ImGui::GetTime();
        double diff = current_time - true_playback_start_time - io.DeltaTime;
        // 1.75 for auto
        if (diff * 1000.0 >= hit_objects_ms[0])
        {
            // printf("now!\n");
            // playback_start_time = 0.0;
            static double delayed_keyup = 0.0;
            static bool last = false;
            if (diff * 1000.0 >= hit_objects_ms[hit_objects_ms_idx])
            {
                printf("hit %d!\n", hit_objects_ms_idx);
                // send_input("S", 1);

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

                hit_objects_ms_idx++;
                if (hit_objects_ms_idx == 414)
                {
                    // printf("end!\n");
                    last = true;
                }
            }
            if (delayed_keyup && ((current_time - delayed_keyup) > 0.0005))
            {
                // printf("keyup!\n");
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
                    playback_start_time = 0.0;
                    true_playback_start_time = 0.0;
                    last = false;
                }
            }
        }

        // static DWORD last_dwFlags = KEYEVENTF_KEYUP;
        // if (diff > 0.517392)
        // {
        //     printf("keypress!\n");
        //     playback_start_time = ImGui::GetTime();

        //     INPUT inputs[1];
        //     inputs[0].type = INPUT_KEYBOARD;
        //     inputs[0].ki.wVk = 0;
        //     inputs[0].ki.wScan = 0;
        //     inputs[0].ki.time = 0;
        //     inputs[0].ki.dwExtraInfo = 0;
        //     inputs[0].ki.dwFlags = last_dwFlags == KEYEVENTF_KEYUP ? 0 : KEYEVENTF_KEYUP;
        //     last_dwFlags = inputs[0].ki.dwFlags;
        //     inputs[0].ki.wVk = 'S';
        //     SendInput(1, inputs, sizeof(INPUT));
        // }
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

    // glPushAttrib(GL_ALL_ATTRIB_BITS);
    // glPushMatrix();
    // GLint viewport[4];
    // glGetIntegerv(GL_VIEWPORT, viewport);
    // glViewport(0, 0, viewport[2], viewport[3]);
    // glMatrixMode(GL_PROJECTION);
    // glLoadIdentity();
    // glOrtho(0, viewport[2], viewport[3], 0, -1, 1);
    // glMatrixMode(GL_MODELVIEW);
    // glLoadIdentity();
    // glDisable(GL_DEPTH_TEST);

    // glColor3f(1.0, 0.0, 0.0);
    // glBegin(GL_QUADS);
    // glVertex2f(x, y);
    // glVertex2f(x + width, y);
    // glVertex2f(x + width, y + height);
    // glVertex2f(x, y + height);
    // glEnd();

    // glPopMatrix();
    // glPopAttrib();

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
