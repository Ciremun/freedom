#pragma once

#include <stdint.h>

#include "parse.h"
#include "window.h"
#include "utility.h"

template <typename T>
Vector2<T> prepare_hitcircle_target(uintptr_t osu_player_ptr, const Circle &circle, Vector2<T> &mouse_position)
{
    uintptr_t osu_manager_ptr = **(uintptr_t **)(osu_player_ptr + 0x8);
    uintptr_t osu_ruleset_ptr = *(uintptr_t *)(osu_manager_ptr + 0x60);
    float mouse_x = *(float *)(osu_ruleset_ptr + 0x80);
    float mouse_y = *(float *)(osu_ruleset_ptr + 0x84);
    mouse_position.x = mouse_x;
    mouse_position.y = mouse_y;
    Vector2 circle_position_on_screen = playfield_to_screen(circle.position);
    Vector2 direction = circle_position_on_screen - mouse_position;
    return direction;
}
