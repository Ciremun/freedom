#pragma once

#include "legacy/hook.h"
#include "legacy/hitobject.h"

#include "ui/config.h"

extern uintptr_t selected_replay_code_start;
extern uintptr_t selected_replay_offset;
extern uintptr_t selected_replay_hook_jump_back;
extern uintptr_t selected_replay_ptr;

extern Hook<Detour32> SelectedReplayHook;

void init_replay();

void update_replay();
void replay_on_beatmap_load();

void enable_replay_hooks();
void disable_replay_hooks();

void notify_on_select_replay();
