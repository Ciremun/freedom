#pragma once

#include <stdint.h>

#include "minhook.h"

#include "ui/debug_log.h"

struct DifficultySetting
{
    bool enabled;
    float value;
    const char *fmt;
    void (*enable)();
    void (*disable)();
};

extern DifficultySetting ar_setting;
extern DifficultySetting cs_setting;
extern DifficultySetting od_setting;

extern uintptr_t on_beatmap_changed_ptr;

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

inline bool is_difficulty_setting_found(DifficultySetting *p);
