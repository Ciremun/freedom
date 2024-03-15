#pragma once

#include <stdint.h>

#include "parse.h"
#include "window.h"
#include "memory.h"
#include "input.h"
#include "struct_offsets.h"

#include "ui/config.h"

extern bool beatmap_loaded;
extern bool mods_updated;
extern bool start_parse_replay;
extern BeatmapData current_beatmap;
extern ReplayData current_replay;

extern float od_window_left_offset;
extern float od_window_right_offset;

extern float od_window;
extern float jumping_window_offset;

extern int wait_hitobjects_min;
extern int wait_hitobjects_max;

void process_hitobject();
bool scene_is_game(Scene *current_scene_ptr);
bool is_playing(uintptr_t audio_time_ptr);
bool is_replay_mode(uintptr_t osu_manager_ptr);
