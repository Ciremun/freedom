#pragma once

#pragma once

#include <stdint.h>
#include <inttypes.h>

#include "minhook/minhook.h"

#include "ui/ui.h"
#include "ui/debug_log.h"
#include "lazer/struct_offsets.h"
#include "lazer/features/difficulty.h"

extern uintptr_t on_beatmap_changed_ptr;

void init_on_beatmap_changed();
void enable_beatmap_changed_hook();
