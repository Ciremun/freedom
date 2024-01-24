#pragma once

#include <stdint.h>

#include "hook.h"
#include "config.h"

extern uintptr_t score_multiplier_code_start;
extern uintptr_t score_multiplier_hook_jump_back;

void init_score_multiplier();

void enable_score_multiplier_hooks();
void disable_score_multiplier_hooks();

void set_score_multiplier();
