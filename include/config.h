#pragma once

#include <windows.h>

#include "imgui.h"
#include "imgui_internal.h"

#include "detours.h"
#include "mem.h"

extern int cfg_font_size;
extern int cfg_spins_per_minute;
extern bool cfg_mod_menu_visible;

const char *get_imgui_ini_filename(HMODULE hMod);
void set_imgui_ini_handler();
