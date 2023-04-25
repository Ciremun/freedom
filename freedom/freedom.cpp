// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#pragma comment(lib, "Winhttp.lib")
#pragma comment(lib, "Opengl32.lib")

#include "stdafx.h"

#include "detours.h"
#include "parse.h"
#include "input.h"
#include "ui.h"
#include "hitobject.h"

HWND g_hwnd = NULL;
HANDLE g_process = NULL;
HMODULE g_module = NULL;

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

HDC hDc = 0;

__declspec(naked) void freedom_update()
{
    // if (!hDc)
        // return wglSwapBuffersGateway(hDc);

    static bool init = false;
    if (!init)
    {
        hDc = wglGetCurrentDC();
        g_hwnd = WindowFromDC(hDc);

// #ifndef NDEBUG
//         AllocConsole();
//         FILE *f;
//         freopen_s(&f, "CONOUT$", "w", stdout);
//         freopen_s(&f, "CONOUT$", "w", stderr);
// #endif // NDEBUG

        g_process = GetCurrentProcess();

        init_ui();
        CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)init_hooks, 0, 0 ,0));

        init = true;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    process_hitobject();

    if (GetAsyncKeyState(VK_F11) & 1)
    {
        cfg_mod_menu_visible = !cfg_mod_menu_visible;
        if (!cfg_mod_menu_visible)
            ImGui::GetIO().MouseDrawCursor = false;
        ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
    }

    if (!cfg_mod_menu_visible)
        goto frame_end;

    update_ui();

    ImGui::GetIO().MouseDrawCursor = ImGui::GetIO().WantCaptureMouse;

frame_end:

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    wglSwapBuffersGateway();
}

DWORD WINAPI freedom_main(HMODULE hModule)
{
    g_module = hModule;

    SwapBuffersHook = Hook<Trampoline32>("wglSwapBuffers", "opengl32.dll", (BYTE *)freedom_update, (BYTE *)&wglSwapBuffersGateway, 5);
    SwapBuffersHook.src += 14;
    SwapBuffersHook.Enable();

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)freedom_main, hModule, 0, 0));
    return TRUE;
}
