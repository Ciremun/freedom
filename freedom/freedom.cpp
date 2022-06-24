#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "CorGuids.lib")

#include "stdafx.h"
#include <atlbase.h>
#include <atlexcept.h>
#include <atlconv.h>

#include "dotnet_data_collector.h"
#include "hook.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
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
        config.SizePixels = 24;
        config.OversampleH = config.OversampleV = 1;
        config.PixelSnapH = true;
        ImFont *default_font = io.Fonts->AddFontDefault(&config);

        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(g_HWND);
        ImGui_ImplOpenGL3_Init();

        ImGuiStyle &style = ImGui::GetStyle();
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

#define BLACK ImVec4(0.05f, 0.05f, 0.05f, 1.0f)
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

        init = true;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Freedom", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

    static bool params_visible = true;
    static uintptr_t osu_auth_base = GetModuleBaseAddress(L"osu!auth.dll");
    static uintptr_t current_song_ptr = internal_multi_level_pointer_dereference(GetCurrentProcess(), osu_auth_base + selected_song_ptr_base_offset, selected_song_ptr_offsets);
    if (current_song_ptr)
    {
        uint32_t song_struct = *(uint32_t *)current_song_ptr;
        static uintptr_t prev_song_name_ptr = 0;
        uintptr_t song_name_ptr = song_struct + 0x80;
        static char song_name_u8[128] = {0};
        if (song_name_ptr != prev_song_name_ptr)
        {
            uint32_t song_name_length = *(uint32_t *)(*(char **)song_name_ptr + 0x4);
            char *song_name_u16 = *(char **)song_name_ptr + 0x8;
            ATL::CW2A utf8((wchar_t *)song_name_u16, CP_UTF8);
            memcpy(song_name_u8, utf8.m_psz, 128);
        }
        prev_song_name_ptr = song_name_ptr;
        ImGui::Text("%s\n", song_name_u8);
        if (ImGui::IsItemClicked())
            params_visible = !params_visible;
    }
    else
    {
        current_song_ptr = internal_multi_level_pointer_dereference(GetCurrentProcess(), osu_auth_base + selected_song_ptr_base_offset, selected_song_ptr_offsets);
    }

    if (params_visible)
    {
        ImGui::Dummy(ImVec2(0.0f, 2.0f));
        ImGui::PushItemWidth(200.0f);
        ImGui::SliderFloat("##AR", &ar_value, 0.0f, 10.0f, "AR: %.1f");
        ImGui::PopItemWidth();
    }

    ImGui::End();

    ImGuiIO &io = ImGui::GetIO();
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
