#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "CorGuids.lib")

#include "stdafx.h"
#include <atlbase.h>
#include <atlconv.h>
#include <atlexcept.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include "imgui_internal.h"
#include "stb_sprintf.h"

#include "dotnet_data_collector.h"
#include "hook.h"
#include "offsets.h"
#include "utility.h"

typedef BOOL(__stdcall *twglSwapBuffers)(HDC hDc);
typedef BOOL(__stdcall *tdefaultGateway)();
twglSwapBuffers wglSwapBuffersGateway;

HWND g_HWND = NULL;
float ar_value = 10.0f;

WNDPROC oWndProc;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK EnumWindowsProcMy(HWND hwnd, LPARAM lParam)
{
    DWORD lpdwProcessId;
    GetWindowThreadProcessId(hwnd, &lpdwProcessId);
    if (lpdwProcessId == lParam)
    {
        g_HWND = hwnd;
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
        EnumWindows(EnumWindowsProcMy, GetCurrentProcessId());
        oWndProc = (WNDPROC)SetWindowLongPtrA(g_HWND, GWLP_WNDPROC, (LONG_PTR)WndProc);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.IniFilename = 0;

        ImFontConfig config;
        config.OversampleH = config.OversampleV = 1;
        config.PixelSnapH = true;

        config.SizePixels = 32;
        io.Fonts->AddFontDefault(&config);

        config.SizePixels = 28;
        io.Fonts->AddFontDefault(&config);

        config.SizePixels = 24;
        font = io.Fonts->AddFontDefault(&config);

        config.SizePixels = 18;
        io.Fonts->AddFontDefault(&config);

        config.SizePixels = 14;
        io.Fonts->AddFontDefault(&config);

        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(g_HWND);
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

    ImGuiIO &io = ImGui::GetIO();

    ImGui::PushFont(font);

    ImGui::Begin("Freedom", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

    static uintptr_t osu_auth_base = GetModuleBaseAddress(L"osu!auth.dll");
    static uintptr_t current_song_ptr = internal_multi_level_pointer_dereference(GetCurrentProcess(), osu_auth_base + selected_song_ptr_base_offset, selected_song_ptr_offsets);
    static char song_name_u8[128] = {0};
    if (current_song_ptr)
    {
        uintptr_t song_name_ptr = *(uint32_t *)current_song_ptr + 0x80;
        static uintptr_t prev_song_name_ptr = 0;
        if (song_name_ptr != prev_song_name_ptr)
        {
            uint32_t song_name_length = *(uint32_t *)(*(char **)song_name_ptr + 0x4);
            char *song_name_u16 = *(char **)song_name_ptr + 0x8;
            ATL::CW2A utf8((wchar_t *)song_name_u16, CP_UTF8);
            memcpy(song_name_u8, utf8.m_psz, 127);
        }
        prev_song_name_ptr = song_name_ptr;
    }
    else
    {
        current_song_ptr = internal_multi_level_pointer_dereference(GetCurrentProcess(), osu_auth_base + selected_song_ptr_base_offset, selected_song_ptr_offsets);
    }

    ImGui::Text("%s", song_name_u8);

    if (ImGui::BeginPopupContextItem("##settings"))
    {
        ImGuiContext &g = *ImGui::GetCurrentContext();
        static char preview_font_size[16] = {0};
        stbsp_snprintf(preview_font_size, 16, "Font Size: %dpx", (int)g.Font->ConfigData->SizePixels);

        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Settings").x) * 0.5f);
        ImGui::Text("Settings");
        ImGui::Dummy(ImVec2(0.0f, 5.0f));

#define ITEM_DISABLED ImVec4(0.50f, 0.50f, 0.50f, 1.00f)
        static bool ar_lock = false;
        if (!ar_lock)
        {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleColor(ImGuiCol_Text, ITEM_DISABLED);
            ImGui::SliderFloat("##AR", &ar_value, 0.0f, 10.0f, "AR: %.1f");
            ImGui::PopStyleColor();
            ImGui::PopItemFlag();
        }
        else
        {
            ImGui::SliderFloat("##AR", &ar_value, 0.0f, 10.0f, "AR: %.1f");
        }
        ImGui::SameLine();
        ImGui::Checkbox("##ar_lock", &ar_lock);

        ImGui::Dummy(ImVec2(0.0f, 5.0f));
        if (ImGui::BeginCombo("##font_size", preview_font_size))
        {
            for (const auto &f : io.Fonts->Fonts)
            {
                char font_size[8] = {0};
                stbsp_snprintf(font_size, 4, "%d", (int)f->ConfigData->SizePixels);
                const bool is_selected = f == font;
                if (ImGui::Selectable(font_size, is_selected))
                    font = f;
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

uintptr_t parse_beatmap_metadata_code_start = 0;
uintptr_t parse_beatmap_metadata_jump_back_146C = 0;
uintptr_t parse_beatmap_metadata_jump_back_14BC = 0;

__declspec(naked) void set_approach_rate_146C()
{
    __asm {
        fstp dword ptr [eax+0x2C]
        mov ebx, ar_value
        mov dword ptr [eax+0x2C], ebx
        jmp [parse_beatmap_metadata_jump_back_146C]
    }
}

__declspec(naked) void set_approach_rate_14BC()
{
    __asm {
        mov eax, dword ptr [ebp-0x00000150]
        fstp dword ptr [eax+0x2C]
        mov ebx, ar_value
        mov dword ptr [eax+0x2C], ebx
        jmp [parse_beatmap_metadata_jump_back_14BC]
    }
}

DWORD WINAPI freedom_main(HMODULE hModule)
{
    // AllocConsole();
    // FILE* f;
    // freopen_s(&f, "CONOUT$", "w", stdout);

    Hook SwapBuffersHook("wglSwapBuffers", "opengl32.dll", (BYTE *)freedom_update, (BYTE *)&wglSwapBuffersGateway, 5);
    SwapBuffersHook.Enable();

    while ((parse_beatmap_metadata_code_start = code_start_for_parse_beatmap_metadata()) == 0)
        Sleep(1000);
    parse_beatmap_metadata_jump_back_146C = parse_beatmap_metadata_code_start + 0x14C5;
    parse_beatmap_metadata_jump_back_14BC = parse_beatmap_metadata_code_start + 0x14C5;

    detour_32((BYTE *)parse_beatmap_metadata_code_start + 0x146C, (BYTE *)&set_approach_rate_146C, 5);
    detour_32((BYTE *)parse_beatmap_metadata_code_start + 0x14BC, (BYTE *)&set_approach_rate_14BC, 9);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)freedom_main,
                                 hModule, 0, nullptr));
    default:
        break;
    }
    return TRUE;
}
