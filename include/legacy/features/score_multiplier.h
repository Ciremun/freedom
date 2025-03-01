#pragma once

#include <stdint.h>

#include "legacy/hook.h"

#include "ui/config.h"

extern uintptr_t score_multiplier_code_start;
extern uintptr_t score_multiplier_offset;
extern uintptr_t score_multiplier_hook_jump_back;

extern Hook<Detour32> ScoreMultiplierHook;

void init_score_multiplier();

void enable_score_multiplier_hooks();
void disable_score_multiplier_hooks();

void set_score_multiplier();
