#pragma once

#include "vector.h"

constexpr Vector2 window_size(1920.0f, 1080.0f);
constexpr float window_ratio = window_size.y / 480.0f;
constexpr Vector2 playfield_size(512.0f * window_ratio, 384.0f * window_ratio);
constexpr Vector2 playfield_position((window_size.x - playfield_size.x) / 2.0f, (window_size.y - playfield_size.y) / 4.0f * 3.0f + (-16.0f * window_ratio));
constexpr float playfield_ratio = playfield_size.y / 384.0f;

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
