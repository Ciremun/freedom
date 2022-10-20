#pragma once

#include <windows.h>

#include "parse.h"

#include <thread>

#define FR_VERSION "v0.8"

extern uintptr_t osu_auth_base;

extern HWND g_hwnd;
extern HMODULE g_module;
extern HANDLE g_process;
