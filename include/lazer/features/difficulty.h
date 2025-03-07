#pragma once

#include <stdint.h>
#include <inttypes.h>

#include "minhook/minhook.h"

#include "ui/ui.h"
#include "ui/debug_log.h"
#include "lazer/struct_offsets.h"

struct DifficultySetting
{
    bool enabled;
    float value;
    const char *label;
    const char *fmt;
};

extern DifficultySetting ar_setting;
extern DifficultySetting cs_setting;
extern DifficultySetting od_setting;
extern DifficultySetting hp_setting;

extern uintptr_t on_beatmap_changed_ptr;

void init_difficulty();
void enable_difficulty_hook();
void disable_difficulty_hook();
