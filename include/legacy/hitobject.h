#pragma once

#include <stdint.h>

#include "legacy/parse.h"
#include "legacy/window.h"
#include "memory.h"
#include "legacy/input.h"
#include "legacy/struct_offsets.h"

#include "ui/config.h"

extern bool beatmap_loaded;
extern bool mods_updated;
extern bool start_parse_replay;
extern BeatmapData current_beatmap;
extern ReplayData current_replay;

void process_hitobject();
bool scene_is_game(Scene *current_scene_ptr);
bool is_playing(uintptr_t audio_time_ptr);
bool is_replay_mode(uintptr_t osu_manager_ptr);
