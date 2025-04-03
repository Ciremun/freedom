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
extern DifficultySetting dr_setting;

bool init_difficulty(uintptr_t base);
