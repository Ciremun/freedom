#pragma once

#include <stdint.h>

#include "hook.h"
#include "config.h"

typedef void(*tHiddenHook)();

extern uintptr_t hom_update_vars_hidden_loc;
extern uintptr_t osu_manager_ptr;

void init_unmod_hidden();
void unmod_hidden_on_beatmap_load();
void enable_hidden_remover_hooks();
void disable_hidden_remover_hooks();

void hk_hom_update_vars_hidden();
