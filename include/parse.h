#pragma once

#include <windows.h>

#include <vector>

#include <stdint.h>

#include "mem.h"
#include "vector.h"
#include "window.h"

enum class Scene
{
    MAIN_MENU = 0,
    EDITOR,
    GAME,
    EXIT,
    EDITOR_BEATMAP_SELECT,
    BEATMAP_SELECT
};

enum class HitObjectType
{
    Circle = 1 << 0,
    Slider = 1 << 1,
    NewCombo = 1 << 2,
    Spinner = 1 << 3,
    ComboOffset = 1 << 4 | 1 << 5 | 1 << 6,
    Hold = 1 << 7
};

inline HitObjectType& operator&= (HitObjectType& a, HitObjectType b) { return reinterpret_cast<HitObjectType&>( reinterpret_cast<int32_t&>(a) &= static_cast<int32_t>(b) ); }
inline HitObjectType operator~ (HitObjectType a) { return static_cast<HitObjectType>( ~static_cast<int32_t>(a) ); }

enum Mods
{
    DoubleTime = 1 << 6,
    HalfTime = 1 << 8
};

struct Circle
{
    int32_t start_time = 0;
    int32_t end_time = 0;
    Vector2<float> position;
    HitObjectType type;
    std::vector<Vector2<float>> curves;
    uint32_t curve_idx = 0;
    bool clicked = false;
};

struct BeatmapData
{
    std::vector<Circle> hit_objects;
    uint32_t hit_object_idx = 0;
    bool ready = false;
    Mods mods;

    void clear();
    Circle& current_circle();
};

bool parse_beatmap(uintptr_t osu_manager_ptr, BeatmapData &beatmap_data);
