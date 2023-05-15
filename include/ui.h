#pragma once

#include <stdlib.h>

#include "detours.h"
#include "freedom.h"
#include "config.h"
#include "font.h"
#include "input.h"
#include "hitobject.h"
#include "log.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include "imgui_internal.h"

#include "stb_sprintf.h"

#define ITEM_DISABLED ImVec4(0.50f, 0.50f, 0.50f, 1.00f)
#define ITEM_UNAVAILABLE ImVec4(0.722f, 0.059f, .0f, 1.00f)
#define BLACK ImVec4(0.05f, 0.05f, 0.05f, 1.0f)
#define WHITE ImVec4(1.0f, 1.0f, 1.0f, 1.0f)
#define BLACK_TRANSPARENT ImVec4(0.05f, 0.05f, 0.05f, 0.7f)
#define PURPLE ImVec4(0.28f, 0.05f, 0.66f, 1.0f)
#define MAGENTA ImVec4(0.34f, 0.04f, 0.68f, 1.0f)

enum class MenuTab
{
    Difficulty = 0,
    Relax,
    Aimbot,
    Timewarp,
    Replay,
    Mods,
    Misc,
    About,
    Debug
};

extern char song_name_u8[256];
extern ImGuiLogger freedom_log;

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void init_ui();
void update_ui();
void destroy_ui();
void parameter_slider(uintptr_t selected_song_ptr, Parameter *p);
