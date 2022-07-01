#pragma once

#include <windows.h>

#include "imgui.h"
#include "imgui_internal.h"

extern bool ar_lock;
extern float ar_value;

const char *get_imgui_ini_filename(HMODULE hMod);
void set_imgui_ini_handler();
