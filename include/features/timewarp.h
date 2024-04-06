#pragma once

#include "hook.h"

#include "ui/config.h"

extern uintptr_t set_playback_rate_code_start;
extern uintptr_t set_playback_rate_jump_back;
extern uintptr_t set_playback_rate_original_mov_addr;

extern uintptr_t check_timewarp_code_start;
extern uintptr_t check_timewarp_value_1;
extern uintptr_t check_timewarp_value_2;

extern uintptr_t check_timewarp_hook_1;
extern uintptr_t check_timewarp_hook_1_jump_back;

extern uintptr_t check_timewarp_hook_2;
extern uintptr_t check_timewarp_hook_2_jump_back;

extern uintptr_t update_timing_code_start;
extern uintptr_t update_timing_ptr_1;
extern uintptr_t update_timing_ptr_2;
extern uintptr_t update_timing_ptr_3;
extern uintptr_t update_timing_ptr_4;

extern Hook<Detour32> SetPlaybackRateHook;
extern Hook<Detour32> CheckTimewarpHook1;
extern Hook<Detour32> CheckTimewarpHook2;

void init_timewarp();

void enable_timewarp_hooks();
void disable_timewarp_hooks();

void set_playback_rate();
void set_check_timewarp_hook_1();
void set_check_timewarp_hook_2();
