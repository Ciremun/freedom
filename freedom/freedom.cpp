#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "CorGuids.lib")
#pragma comment(lib, "Winhttp.lib")

#include "stdafx.h"

#include "detours.h"
#include "parse.h"
#include "input.h"
#include "ui.h"
#include "hitobject.h"

HWND g_hwnd = NULL;
HANDLE g_process = NULL;
HMODULE g_module = NULL;
uintptr_t osu_auth_base = NULL;

static void unload_module()
{
    Sleep(2000);
    VirtualFree(wglSwapBuffersGateway, 0, MEM_RELEASE);
    FreeLibrary(g_module);
}

void unload_freedom()
{
    destroy_ui();
    destroy_hooks();
    std::thread(unload_module).detach();
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
    static bool init = false;
    if (!init)
    {
        EnumWindows(find_osu_window, GetCurrentProcessId());

        if (!IsWindowVisible(g_hwnd))
            return wglSwapBuffersGateway(hDc);

// #ifndef NDEBUG
//         AllocConsole();
//         FILE *f;
//         freopen_s(&f, "CONOUT$", "w", stdout);
//         freopen_s(&f, "CONOUT$", "w", stderr);
// #endif // NDEBUG

        g_process = GetCurrentProcess();
        osu_auth_base = GetModuleBaseAddress(L"osu!auth.dll");

        init_ui();
        std::thread(init_hooks).detach();

        calc_playfield(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

        init = true;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiIO &io = ImGui::GetIO();

    process_hitobject();

    if (GetAsyncKeyState(VK_F11) & 1)
    {
        cfg_mod_menu_visible = !cfg_mod_menu_visible;
        if (!cfg_mod_menu_visible)
            io.MouseDrawCursor = false;
        ImGui::SaveIniSettingsToDisk(io.IniFilename);
    }

    if (!cfg_mod_menu_visible)
        goto frame_end;

    update_ui();

    io.MouseDrawCursor = io.WantCaptureMouse;

frame_end:

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return wglSwapBuffersGateway(hDc);
}

DWORD WINAPI freedom_main(HMODULE hModule)
{
    g_module = hModule;

    SwapBuffersHook = Hook<Trampoline32>("wglSwapBuffers", "opengl32.dll", (BYTE *)freedom_update, (BYTE *)&wglSwapBuffersGateway, 5);
    SwapBuffersHook.Enable();

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)freedom_main, hModule, 0, 0));
    return TRUE;
}
