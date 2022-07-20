#pragma once

#include <windows.h>

#include "enums.h"
#include "file.h"
#include "parse.h"

extern uintptr_t osu_auth_base;
extern bool beatmap_loaded;
extern bool start_parse_beatmap;
extern Scene current_scene;
extern BeatmapData current_beatmap;

extern HMODULE g_module;
extern HANDLE g_process;
