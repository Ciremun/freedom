#pragma once

#include <stdint.h>

#include "legacy/hook.h"
#include "legacy/scan.h"
#include "legacy/struct_offsets.h"

#include "ui/config.h"

typedef void(*tHiddenHook)();

extern uintptr_t hom_update_vars_hidden_loc;
extern uintptr_t osu_manager_ptr;

extern Hook<Trampoline32> HiddenHook;

void init_unmod_hidden();
void unmod_hidden_on_beatmap_load();
void enable_hidden_remover_hooks();
void disable_hidden_remover_hooks();

void hk_hom_update_vars_hidden();
