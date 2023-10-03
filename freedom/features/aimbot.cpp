#include "features/aimbot.h"

static bool target_first_circle = true;

static float fraction_of_the_distance = 0.0f;
static Vector2 direction(0.0f, 0.0f);
static Vector2 mouse_position(0.0f, 0.0f);

void update_aimbot(Circle &circle, const int32_t audio_time)
{
    if (cfg_aimbot_lock)
    {
        if (fraction_of_the_distance)
        {
            if (fraction_of_the_distance > 1.0f)
            {
                fraction_of_the_distance = 0.0f;
            }
            else
            {
                Vector2 next_mouse_position = mouse_position + direction * fraction_of_the_distance;
                move_mouse_to(next_mouse_position.x, next_mouse_position.y);
                fraction_of_the_distance += cfg_fraction_modifier;
            }
        }
        if (target_first_circle)
        {
            direction = prepare_hitcircle_target(osu_manager_ptr, circle.position, mouse_position);
            fraction_of_the_distance = cfg_fraction_modifier;
            target_first_circle = false;
        }
        if (audio_time >= circle.start_time)
        {
            if (circle.type == HitObjectType::Slider)
            {
                uintptr_t osu_manager = *(uintptr_t *)(osu_manager_ptr);
                uintptr_t hit_manager_ptr = *(uintptr_t *)(osu_manager + OSU_MANAGER_HIT_MANAGER_OFFSET);
                uintptr_t hit_objects_list_ptr = *(uintptr_t *)(hit_manager_ptr + OSU_HIT_MANAGER_HIT_OBJECTS_LIST_OFFSET);
                uintptr_t hit_objects_list_items_ptr = *(uintptr_t *)(hit_objects_list_ptr + 0x4);
                uintptr_t hit_object_ptr = *(uintptr_t *)(hit_objects_list_items_ptr + 0x8 + 0x4 * current_beatmap.hit_object_idx);
                uintptr_t animation_ptr = *(uintptr_t *)(hit_object_ptr + OSU_HIT_OBJECT_ANIMATION_OFFSET);
                float slider_ball_x = *(float *)(animation_ptr + OSU_ANIMATION_SLIDER_BALL_X_OFFSET);
                float slider_ball_y = *(float *)(animation_ptr + OSU_ANIMATION_SLIDER_BALL_Y_OFFSET);
                Vector2<float> slider_ball(slider_ball_x, slider_ball_y);
                direction = prepare_hitcircle_target(osu_manager_ptr, slider_ball, mouse_position);
                fraction_of_the_distance = cfg_fraction_modifier;
            }
            if (circle.type == HitObjectType::Spinner)
            {
                auto& center = circle.position;
                static const float radius = 60.0f;
                static const float PI = 3.14159f;
                static float angle = .0f;
                Vector2 next_point_on_circle(center.x + radius * cosf(angle), center.y + radius * sinf(angle));
                direction = prepare_hitcircle_target(osu_manager_ptr, next_point_on_circle, mouse_position);
                fraction_of_the_distance = 1.0f;
                angle > 2 * PI ? angle = 0 : angle += cfg_spins_per_minute / (2 * PI) * ImGui::GetIO().DeltaTime;
            }
        }
    }
}

void aimbot_on_beatmap_load()
{
    target_first_circle = true;
}

void advance_aimbot()
{
    if (cfg_aimbot_lock)
    {
        Circle& next_circle = current_beatmap.current_circle();
        switch (next_circle.type)
        {
            case HitObjectType::Circle:
            case HitObjectType::Slider:
            case HitObjectType::Spinner:
            {
                direction = prepare_hitcircle_target(osu_manager_ptr, next_circle.position, mouse_position);
                fraction_of_the_distance = cfg_fraction_modifier;
            } break;
        }
    }
}