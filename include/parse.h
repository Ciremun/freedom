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
    SLIDER,
    SPINNER
};

struct HitCircle
{
    int32_t time;
};

struct Slider
{
    int32_t time;
    int32_t slides;
    double length;
};

struct Spinner
{
    int32_t time;
    int32_t end_time;
};

struct Circle
{
    CircleType type;
    union {
        Slider slider;
        HitCircle hit_circle;
        Spinner spinner;
    };
};

struct TimingPoint
{
    int32_t time;
    double beat_length;
    bool uninherited;
};

struct BeatmapData
{
    std::vector<TimingPoint> timing_points;
    std::vector<Circle> hit_objects;
    uint32_t hit_object_idx = 0;
    uint32_t timing_point_idx = 0;
    double slider_multiplier = 0.0;
    bool parsed_successfully = false;

    void clear();
    Circle current_circle();
    TimingPoint current_timing_point();
};

bool parse_beatmap(const wchar_t *beatmap_path, BeatmapData &beatmap_data);
