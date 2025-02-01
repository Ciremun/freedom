#pragma once

#include <stdint.h>

#include "memory.h"
#include "vector.h"
#include "freedom.h"

extern uintptr_t window_manager_code_start;
extern uintptr_t window_manager_offset;
extern uintptr_t window_manager_ptr;

extern Vector2<float> window_size;
extern Vector2<float> playfield_size;
extern Vector2<float> playfield_position;
extern Vector2<float> client_offset;
extern Vector2<float> primary_monitor;

extern float window_ratio;
extern float playfield_ratio;

template <typename T>
Vector2<T> screen_to_playfield(Vector2<T> screen_coords)
{
    return (screen_coords - playfield_position) / playfield_ratio;
}

template <typename T>
Vector2<T> playfield_to_screen(Vector2<T> playfield_coords)
{
    return (playfield_coords * playfield_ratio) + playfield_position;
}

void calc_playfield_manual(float window_x, float window_y);
bool calc_playfield_from_window();
