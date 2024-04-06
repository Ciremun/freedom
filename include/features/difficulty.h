#pragma once

#include <stdint.h>

#include "hook.h"
#include "scan.h"
#include "struct_offsets.h"

struct Parameter
{
    bool lock;
    float value;
    float calculated_value;
    uintptr_t offset;
    const char *slider_fmt;
    const char *error_message;
    void (*enable)();
    void (*disable)();
    void (*apply_mods)();
    bool found = false;
};

extern Parameter ar_parameter;
extern Parameter cs_parameter;
extern Parameter od_parameter;

extern Hook<Detour32> ApproachRateHook1;
extern Hook<Detour32> ApproachRateHook2;
extern Hook<Detour32> ApproachRateHook3;

extern Hook<Detour32> CircleSizeHook1;
extern Hook<Detour32> CircleSizeHook2;
extern Hook<Detour32> CircleSizeHook3;

extern Hook<Detour32> OverallDifficultyHook1;
extern Hook<Detour32> OverallDifficultyHook2;

extern uintptr_t parse_beatmap_code_start;

extern uintptr_t approach_rate_offsets[3];
extern uintptr_t ar_hook_jump_back;

extern uintptr_t circle_size_offsets[3];
extern uintptr_t cs_hook_jump_back;

extern uintptr_t overall_difficulty_offsets[2];
extern uintptr_t od_hook_jump_back;

void init_difficulty();

void enable_ar_hooks();
void disable_ar_hooks();
void apply_mods_ar();

void enable_cs_hooks();
void disable_cs_hooks();
void apply_mods_cs();

void enable_od_hooks();
void disable_od_hooks();
void apply_mods_od();

void set_approach_rate();
void set_approach_rate_2();
void set_circle_size();
void set_overall_difficulty();
