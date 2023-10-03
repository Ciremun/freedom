#pragma once

#include "vector.h"
#include "config.h"

void update_aimbot(Circle &circle, const int32_t audio_time);
void aimbot_on_beatmap_load();
void advance_aimbot();

template <typename T>
Vector2<T> prepare_hitcircle_target(uintptr_t osu_manager_ptr, const Vector2<float> &position, Vector2<T> &mouse_position)
{
    uintptr_t osu_manager = *(uintptr_t *)(osu_manager_ptr);
    uintptr_t osu_ruleset_ptr = *(uintptr_t *)(osu_manager + OSU_MANAGER_RULESET_PTR_OFFSET);
    float mouse_x = *(float *)(osu_ruleset_ptr + OSU_RULESET_MOUSE_X_OFFSET);
    float mouse_y = *(float *)(osu_ruleset_ptr + OSU_RULESET_MOUSE_Y_OFFSET);
    mouse_position.x = mouse_x;
    mouse_position.y = mouse_y;
    Vector2 circle_position_on_screen = playfield_to_screen(position);
    Vector2 direction = circle_position_on_screen - mouse_position;
    return direction;
}
