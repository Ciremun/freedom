#pragma once

#include <windows.h>

#include <stdint.h>

#include <vector>

#include "hook.h"
#include "signatures.h"
#include "hitobject.h"
#include "mem.h"
#include "prejit.h"

#include "imgui.h"

typedef void(*twglSwapBuffers)();

struct Parameter
{
    bool lock;
    float value;
    uintptr_t offset;
    const char *slider_fmt;
    const char *error_message;
    void (*enable)();
    void (*disable)();
    bool found = false;
};

extern Parameter ar_parameter;
extern Parameter cs_parameter;
extern Parameter od_parameter;

extern bool cfg_relax_lock;
extern bool cfg_aimbot_lock;

extern uintptr_t parse_beatmap_code_start;

extern uintptr_t approach_rate_offsets[3];
extern uintptr_t ar_hook_jump_back;

extern uintptr_t circle_size_offsets[3];
extern uintptr_t cs_hook_jump_back;

extern uintptr_t overall_difficulty_offsets[2];
extern uintptr_t od_hook_jump_back;

extern uintptr_t beatmap_onload_code_start;
extern uintptr_t beatmap_onload_offset;
extern uintptr_t beatmap_onload_hook_jump_back;

extern uintptr_t score_multiplier_code_start;
extern uintptr_t score_multiplier_hook_jump_back;

extern DWORD discord_rich_presence_code_start;
extern DWORD discord_rich_presence_jump_back;
extern DWORD discord_rich_presence_state_string_ptr;
extern DWORD discord_rich_presence_large_text_string_ptr;
extern DWORD discord_rich_presence_small_text_string_ptr;

extern uintptr_t current_scene_code_start;
extern uintptr_t current_scene_offset;
extern Scene *current_scene_ptr;

extern uintptr_t selected_song_code_start;
extern uintptr_t selected_song_ptr;

extern uintptr_t audio_time_code_start;
extern uintptr_t audio_time_ptr;

extern uintptr_t osu_manager_code_start;
extern uintptr_t osu_manager_ptr;

extern uintptr_t binding_manager_code_start;
extern uintptr_t binding_manager_ptr;

extern uintptr_t selected_replay_code_start;
extern uintptr_t selected_replay_offset;
extern uintptr_t selected_replay_hook_jump_back;
extern uintptr_t selected_replay_ptr;

extern uintptr_t window_manager_code_start;
extern uintptr_t window_manager_offset;
extern uintptr_t window_manager_ptr;

extern uintptr_t update_timing_code_start;
extern uintptr_t update_timing_offset_1;
extern uintptr_t update_timing_offset_2;
extern uintptr_t update_timing_offset_3;

extern uintptr_t set_playback_rate_code_start;
extern uintptr_t set_playback_rate_jump_back;

extern uintptr_t osu_client_id_code_start;
extern char osu_client_id[64];

extern uintptr_t osu_username_code_start;
extern char osu_username[32];

extern uintptr_t dispatch_table_id;
extern uintptr_t nt_user_send_input_dispatch_table_id_found;

extern twglSwapBuffers wglSwapBuffersGateway;

extern Hook<Trampoline32> SwapBuffersHook;

extern float memory_scan_progress;

void init_hooks();

void enable_ar_hooks();
void disable_ar_hooks();

void enable_cs_hooks();
void disable_cs_hooks();

void enable_od_hooks();
void disable_od_hooks();

void enable_notify_hooks();
void disable_notify_hooks();

void enable_replay_hooks();
void disable_replay_hooks();

void set_approach_rate();
void set_approach_rate_2();
void set_circle_size();
void set_overall_difficulty();
void set_score_multiplier();
void set_discord_rich_presence();

void notify_on_beatmap_load();
void notify_on_select_replay();

void enable_score_multiplier_hooks();
void disable_score_multiplier_hooks();

void enable_discord_rich_presence_hooks();
void disable_discord_rich_presence_hooks();

void enable_flashlight_hooks();
void disable_flashlight_hooks();

void enable_timewarp_hooks();
void disable_timewarp_hooks();

void enable_nt_user_send_input_patch();
void disable_nt_user_send_input_patch();

void set_playback_rate();

void destroy_hooks();
