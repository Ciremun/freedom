#pragma once

#include <stdlib.h>

#include <d3d9.h>

#include "scan.h"
#include "font.h"
#include "freedom.h"
#include "config.h"
#include "input.h"
#include "hitobject.h"
#include "log.h"
#include "clrhost.h"
#include "colors.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "imgui_internal.h"

#include "stb_sprintf.h"

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
extern ImGuiLogger debug_log;
extern bool show_debug_log_window;

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void init_ui();
void init_ui(IDirect3DDevice9* pDevice);
void init_imgui_styles();
void init_imgui_fonts();
void update_ui();
void destroy_ui();
void parameter_slider(uintptr_t selected_song_ptr, Parameter *p);
void draw_debug_log();
