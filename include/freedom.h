#pragma once

#include <windows.h>

#include "parse.h"

#include <thread>

#define FR_VERSION "v0.93.2"

extern HWND g_hwnd;
extern HMODULE g_module;
extern HANDLE g_process;

void unload_dll();
