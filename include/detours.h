#pragma once

#include <windows.h>

#include "config.h"
#include "hook.h"
#include "dotnet_data_collector.h"

typedef BOOL(__stdcall *twglSwapBuffers)(HDC hDc);
typedef void(__stdcall *void_trampoline)();

extern twglSwapBuffers wglSwapBuffersGateway;
extern void_trampoline empty_trampoline;

extern bool ar_offsets_found;

extern uintptr_t parse_beatmap_metadata_code_start;
extern uintptr_t ar_hook_jump_back;

extern Hook SwapBuffersHook;

void try_find_hook_offsets();

void init_ar_hooks();
void enable_ar_hooks();
void disable_ar_hooks();

void set_approach_rate_1();
void set_approach_rate_2();
