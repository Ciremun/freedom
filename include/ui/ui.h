#pragma once

#include <stdlib.h>

#include <d3d9.h>
#include <d3d11.h>

#include "ui/font.h"
#include "ui/config.h"
#include "ui/debug_log.h"
#include "ui/colors.h"

#include "imgui.h"
#ifndef FR_LAZER
#include "imgui_impl_opengl3.h"
#include "imgui_impl_dx9.h"
#endif // FR_LAZER
#include "imgui_impl_dx11.h"
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

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#ifndef FR_LAZER
void init_ui();
void init_ui(IDirect3DDevice9* pDevice);
#else
void init_ui(ID3D11Device* p_device, ID3D11DeviceContext* p_context);
#endif // FR_LAZER
void init_imgui_styles();
void init_imgui_fonts();
void update_ui();
void destroy_ui();
void draw_debug_log();
