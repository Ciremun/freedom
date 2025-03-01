#pragma once

#include <windows.h>
#include <winhttp.h>

#include <vector>

#include <stdint.h>
#include <math.h>

#include "memory.h"
#include "legacy/vector.h"
#include "legacy/window.h"
#include "legacy/struct_offsets.h"
#include "ui/debug_log.h"

#include "stb_sprintf.h"

enum class Scene : int32_t
{
    MAIN_MENU = 0,
    EDITOR,
    GAME,
    EXIT,
    EDITOR_BEATMAP_SELECT,
    BEATMAP_SELECT,
    BEATMAP_SELECT_DRAWINGS,
    REPLAY_PREVIEW,
};

enum class HitObjectType : int32_t
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

enum Mods : int32_t
{
    None = 0,
    NoFail = 1 << 0,
    Easy = 1 << 1,
    TouchDevice = 1 << 2,
    Hidden = 1 << 3,
    HardRock = 1 << 4,
    SuddenDeath = 1 << 5,
    DoubleTime = 1 << 6,
    Relax = 1 << 7,
    HalfTime = 1 << 8,
    Nightcore = 1 << 9,
    Flashlight = 1 << 10,
    Autoplay = 1 << 11,
    SpunOut = 1 << 12,
    Relax2 = 1 << 13,
    Perfect = 1 << 14,
    Key4 = 1 << 15,
    Key5 = 1 << 16,
    Key6 = 1 << 17,
    Key7 = 1 << 18,
    Key8 = 1 << 19,
    FadeIn = 1 << 20,
    Random = 1 << 21,
    Cinema = 1 << 22,
    Target = 1 << 23,
    Key9 = 1 << 24,
    KeyCoop = 1 << 25,
    Key1 = 1 << 26,
    Key3 = 1 << 27,
    Key2 = 1 << 28,
    ScoreV2 = 1 << 29,
};

struct Circle
{
    int32_t start_time = 0;
    int32_t end_time = 0;
    bool clicked = false;
    HitObjectType type;
    Vector2<float> position;
};

struct BeatmapData
{
    std::vector<Circle> hit_objects;
    uint32_t hit_object_idx = 0;
    bool ready = false;
    float hit_object_radius = .0f;
    float scaled_hit_object_radius = .0f;
    Mods mods;

    void clear();
    Circle& current_circle();
};

enum ReplayKeys : uint32_t
{
    NO_KEY = 0,
    M1_KEY = 1,
    M2_KEY = 2,
    K1_KEY = 4,
    K2_KEY = 8,
    SMOKE_KEY = 16,
};

struct ReplayEntryData
{
    int64_t ms_since_last_frame = 0;
    Vector2<float> position;
    uint32_t keypresses = 0;
};

struct ReplayData
{
    std::vector<ReplayEntryData> entries;
    size_t entries_idx = 0;
    int64_t replay_ms = 0;
    bool ready = false;

    char song_name_u8[256] = "Open Replay Preview in-game to Select a Replay";
    char author[32] = "Replay Author";
    char mods[64] = "Mods";
    float accuracy = .0f;
    uint32_t combo = 0;

    void clear();
    void toggle_hardrock();
    ReplayEntryData& current_entry();
};

char *mods_to_string(Mods &mods, char *buffer);
bool parse_beatmap(uintptr_t osu_manager_ptr, BeatmapData &beatmap_data);
bool parse_replay(uintptr_t selected_replay_ptr, ReplayData &replay);
