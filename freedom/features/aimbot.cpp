#include "features/aimbot.h"

float elapsed_lerp = 0;

static inline float lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

static inline void move_mouse_to_target(const Vector2<float> &target, const Vector2<float> &cursor_pos)
{
    Vector2 target_on_screen = playfield_to_screen(target);
    if (elapsed_lerp < cfg_fraction_modifier)
    {
        float x = lerp(cursor_pos.x, target_on_screen.x, elapsed_lerp / cfg_fraction_modifier);
        float y = lerp(cursor_pos.y, target_on_screen.y, elapsed_lerp / cfg_fraction_modifier);
        elapsed_lerp += ImGui::GetIO().DeltaTime;
        move_mouse_to(x, y);
    }
    else
        move_mouse_to(target_on_screen.x, target_on_screen.y);
}

void update_aimbot(Circle &circle, const int32_t audio_time)
{
    if (!cfg_aimbot_lock)
        return;

    Vector2 cursor_pos = mouse_position();

    if (circle.type == HitObjectType::Circle)
    {
        move_mouse_to_target(circle.position, cursor_pos);
    }
    else if (circle.type == HitObjectType::Slider)
    {
        uintptr_t osu_manager = *(uintptr_t *)(osu_manager_ptr);
        if (!osu_manager) { FR_ERROR("Aimbot: osu_manager"); return; }
        uintptr_t hit_manager_ptr = *(uintptr_t *)(osu_manager + OSU_MANAGER_HIT_MANAGER_OFFSET);
        if (!hit_manager_ptr) { FR_ERROR("Aimbot: hit_manager_ptr"); return; }
        uintptr_t hit_objects_list_ptr = *(uintptr_t *)(hit_manager_ptr + OSU_HIT_MANAGER_HIT_OBJECTS_LIST_OFFSET);
        if (!hit_objects_list_ptr) { FR_ERROR("Aimbot: hit_objects_list_ptr"); return; }
        uintptr_t hit_objects_list_items_ptr = *(uintptr_t *)(hit_objects_list_ptr + 0x4);
        if (!hit_objects_list_items_ptr) { FR_ERROR("Aimbot: hit_objects_list_items_ptr"); return; }
        uintptr_t hit_object_ptr = *(uintptr_t *)(hit_objects_list_items_ptr + 0x8 + 0x4 * current_beatmap.hit_object_idx);
        if (!hit_object_ptr) { FR_ERROR("Aimbot: hit_object_ptr"); return; }
        uintptr_t animation_ptr = *(uintptr_t *)(hit_object_ptr + OSU_HIT_OBJECT_ANIMATION_OFFSET);
        if (!animation_ptr) { FR_ERROR("Aimbot: animation_ptr"); return; }
        float slider_ball_x = *(float *)(animation_ptr + OSU_ANIMATION_SLIDER_BALL_X_OFFSET);
        float slider_ball_y = *(float *)(animation_ptr + OSU_ANIMATION_SLIDER_BALL_Y_OFFSET);
        Vector2 slider_ball(slider_ball_x, slider_ball_y);
        move_mouse_to_target(slider_ball, cursor_pos);
    }
    else if (circle.type == HitObjectType::Spinner && audio_time >= circle.start_time)
    {
        auto& center = circle.position;
        constexpr float radius = 60.0f;
        constexpr float PI = 3.14159f;
        static float angle = .0f;
        Vector2 next_point_on_circle(center.x + radius * cosf(angle), center.y + radius * sinf(angle));
        move_mouse_to_target(next_point_on_circle, cursor_pos);
        float three_pi = 3 * PI;
        if (cfg_timewarp_enabled)
        {
            auto timewarp_playback_rate = cfg_timewarp_playback_rate / 100.0;
            if (timewarp_playback_rate > 1.0)
                three_pi /= timewarp_playback_rate;
        }
        angle > 2 * PI ? angle = 0 : angle += cfg_spins_per_minute / three_pi * ImGui::GetIO().DeltaTime;
    }
}

void aimbot_on_beatmap_load()
{
    if (cfg_aimbot_lock)
    {
        elapsed_lerp = 0;
    }
}

void aimbot_on_advance_hit_object()
{
    elapsed_lerp = 0;
}
