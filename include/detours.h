#pragma once

#include <windows.h>

#include <stdint.h>

#include "hook.h"
#include "dotnet_data_collector.h"

typedef BOOL(__stdcall *twglSwapBuffers)(HDC hDc);
typedef void(__stdcall *void_trampoline)();

struct Parameter
{
    bool lock;
    float value;
    uintptr_t offset;
    const char *slider_fmt;
    const char *error_message;
    void (*enable)();
    void (*disable)();
    bool found = false;
};

extern Parameter ar_parameter;
extern Parameter cs_parameter;
extern Parameter od_parameter;

extern twglSwapBuffers wglSwapBuffersGateway;
extern void_trampoline empty_trampoline;

extern Hook SwapBuffersHook;

void try_find_hook_offsets();

void init_hooks();

void enable_ar_hooks();
void disable_ar_hooks();

void enable_cs_hooks();
void disable_cs_hooks();

void enable_od_hooks();
void disable_od_hooks();

void set_approach_rate();
void set_circle_size();
void set_overall_difficulty();
