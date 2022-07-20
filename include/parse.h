#pragma once

#include <vector>

#include <stdint.h>

#include "file.h"
#include "utility.h"

enum class BeatmapFileSection
{
    UNKNOWN = 0,
    TIMING_POINTS,
    HITOBJECTS
};

enum class CircleType
{
    POINT = 0,
    SLIDER
};

struct HitCircle
{
    int32_t time;
};

struct Slider
{
    int32_t time;
    float length;
};

struct Circle
{
    CircleType type;
    union {
        Slider slider;
        HitCircle hit_circle;
    };
};

struct TimingPoint
{
    int32_t time;
    float beat_length;
    bool uninherited;
};

struct BeatmapData
{
    std::vector<TimingPoint> timing_points;
    std::vector<Circle> hit_objects;
    uint32_t hit_object_idx = 0;
    float slider_multiplier = 0.0f;
    bool parsed_successfully = false;

    void clear();
    Circle current_circle();
};

bool parse_beatmap(const wchar_t *beatmap_path, BeatmapData &beatmap_data);
