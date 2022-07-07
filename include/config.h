#pragma once

#include <windows.h>

#include "imgui.h"
#include "imgui_internal.h"

extern bool cfg_ar_lock;
extern float cfg_ar_value;

extern bool cfg_cs_lock;
extern float cfg_cs_value;

extern bool cfg_mod_menu_visible;
extern int cfg_font_size;

const char *get_imgui_ini_filename(HMODULE hMod);
void set_imgui_ini_handler();
