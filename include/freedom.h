#pragma once

#include <windows.h>

#include "parse.h"

#include <thread>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH Unknown
#endif // GIT_COMMIT_HASH

#define FR_VERSION "v0.94.3 DEV [" STR(GIT_COMMIT_HASH) "]"

extern HWND g_hwnd;
extern HMODULE g_module;
extern HANDLE g_process;

void unload_dll();
