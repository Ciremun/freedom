#pragma once

#include <windows.h>

typedef BOOL(__stdcall *twglSwapBuffers)(HDC hDc);
typedef void(__stdcall *void_trampoline)();

extern twglSwapBuffers wglSwapBuffersGateway;
extern void_trampoline ar_trampoline;

extern uintptr_t parse_beatmap_metadata_code_start;
extern uintptr_t parse_beatmap_metadata_jump_back;

bool init_ar_hooks();
void enable_ar_hooks();
void disable_ar_hooks();

void set_approach_rate_1();
void set_approach_rate_2();
