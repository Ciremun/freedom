#pragma once

#include <windows.h>

#include <vector>

#include <stdint.h>

#include "utility.h"
#include "offsets.h"

struct Circle
{
    int32_t start_time;
    int32_t end_time;
};

struct BeatmapData
{
    std::vector<Circle> hit_objects;
    uint32_t hit_object_idx = 0;
    bool ready = false;

    void clear();
    Circle current_circle();
};

bool parse_beatmap(HANDLE hProc, uintptr_t osu_auth_base, BeatmapData &beatmap_data);
