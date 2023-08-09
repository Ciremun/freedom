// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "hitobject.h"

BeatmapData current_beatmap;
ReplayData current_replay;

bool beatmap_loaded = false;
bool start_parse_replay = false;
bool target_first_circle = true;

static inline bool scene_is_game(Scene *current_scene_ptr)
{
    if (current_scene_ptr == 0) return false;
    return *current_scene_ptr == Scene::GAME;
}

static inline bool is_playing(uintptr_t audio_time_ptr)
{
    if (audio_time_ptr == 0) return false;
    return *(bool *)(audio_time_ptr + 0x30);
}

bool is_replay_mode(uintptr_t osu_manager_ptr)
{
    if (osu_manager_ptr == 0) return false;
    uintptr_t osu_manager = *(uintptr_t *)(osu_manager_ptr);
    if (osu_manager == 0) return false;
    return *(bool *)(osu_manager + 0x17B);
}

void process_hitobject()
{
    static char current_click = cfg_relax_style == 'a' ? right_click[0] : left_click[0];

    if (beatmap_loaded)
    {
        parse_beatmap(osu_manager_ptr, current_beatmap);
        target_first_circle = true;
        current_click = cfg_relax_style == 'a' ? right_click[0] : left_click[0];

        if (current_replay.ready)
        {
            current_replay.replay_ms = 0;
            current_replay.entries_idx = 0;
        }

        if (cfg_flashlight_enabled && osu_manager_ptr)
        {
            uintptr_t osu_manager = *(uintptr_t *)(osu_manager_ptr);
            if (osu_manager)
            {
                uintptr_t osu_ruleset_ptr = *(uintptr_t *)(osu_manager + 0x68);
                if (osu_ruleset_ptr)
                {
                    uintptr_t flashlight_sprite_manager = *(uintptr_t *)(osu_ruleset_ptr + 0x54);
                    if (flashlight_sprite_manager)
                        *(float *)(flashlight_sprite_manager + 0x28) = .0f;
                }
            }
        }

        if (cfg_hidden_remover_enabled && osu_manager_ptr)
        {
            uintptr_t osu_manager = *(uintptr_t *)(osu_manager_ptr);
            if (osu_manager)
            {
                uintptr_t hit_manager_ptr = *(uintptr_t *)(osu_manager + 0x48);
                uintptr_t mods_ptr = *(uintptr_t *)(hit_manager_ptr + 0x34);
                *(int32_t *)(mods_ptr + 0x0C) = hom_mods_original_value;
                hom_mods_original_value = 0;
            }
        }

        beatmap_loaded = false;
    }

    if (start_parse_replay)
    {
        std::thread(parse_replay, selected_replay_ptr, std::ref(current_replay)).detach();
        start_parse_replay = false;
    }

    if (cfg_replay_enabled && (cfg_replay_aim || cfg_replay_keys) && scene_is_game(current_scene_ptr) && current_replay.ready && is_playing(audio_time_ptr) && !is_replay_mode(osu_manager_ptr))
    {
        int32_t audio_time = *(int32_t *)audio_time_ptr;
        ReplayEntryData &entry = current_replay.current_entry();
        if (audio_time >= current_replay.replay_ms + entry.ms_since_last_frame)
        {
            static bool left = false;
            static bool right = false;
            if (current_replay.entries_idx < current_replay.entries.size())
            {
                if (cfg_replay_aim && entry.position.x > 0 && entry.position.y > 0)
                    move_mouse_to(entry.position.x, entry.position.y);
                if (cfg_replay_keys)
                {
                    switch (entry.keypresses)
                    {
                        case ReplayKeys::KEY_LEFT: {
                            if (!left) { send_keyboard_input(left_click[0], 0); left = true; }
                            if (right) { send_keyboard_input(right_click[0], KEYEVENTF_KEYUP); right = false; }
                        } break;
                        case ReplayKeys::KEY_RIGHT: {
                            if (left)   { send_keyboard_input(left_click[0], KEYEVENTF_KEYUP); left = false; }
                            if (!right) { send_keyboard_input(right_click[0], 0); right = true; }
                        } break;
                        case ReplayKeys::NO_KEY: {
                            if (left)  { send_keyboard_input(left_click[0], KEYEVENTF_KEYUP); left = false; }
                            if (right) { send_keyboard_input(right_click[0], KEYEVENTF_KEYUP); right = false; }
                        } break;
                        case ReplayKeys::KEY_LEFT_AND_RIGHT: {
                            if (!left)  { send_keyboard_input(left_click[0], 0); left = true; }
                            if (!right) { send_keyboard_input(right_click[0], 0); right = true; }
                        } break;
                        default:
                            break;
                    }
                }
                current_replay.replay_ms += entry.ms_since_last_frame;
                current_replay.entries_idx++;
            }
            else
            {
                if (cfg_replay_keys)
                {
                    if (left)  { send_keyboard_input(left_click[0], KEYEVENTF_KEYUP); left = false; }
                    if (right) { send_keyboard_input(right_click[0], KEYEVENTF_KEYUP); right = false; }
                }
                current_replay.replay_ms = 0;
                current_replay.entries_idx = 0;
            }
        }
    }

    static double keydown_time = 0.0;
    static double keyup_delay = 0.0;
    static float fraction_of_the_distance = 0.0f;
    static Vector2 direction(0.0f, 0.0f);
    static Vector2 mouse_position(0.0f, 0.0f);
    if ((cfg_relax_lock || cfg_aimbot_lock) && scene_is_game(current_scene_ptr) && current_beatmap.ready && is_playing(audio_time_ptr) && !is_replay_mode(osu_manager_ptr))
    {
        int32_t audio_time = *(int32_t *)audio_time_ptr;
        Circle& circle = current_beatmap.current_circle();
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
        }
        static const auto rand_range_f = [](float f_min, float f_max) -> float
        {
            float scale = rand() / (float)RAND_MAX;
            return f_min + scale * (f_max - f_min);
        };
        static float act_before_ms = .0f;
        if (cfg_relax_checks_od && (act_before_ms == .0f))
            act_before_ms = rand_range_f(.1337f, current_beatmap.od_window - .5f);
        if (audio_time + act_before_ms >= circle.start_time)
        {
            if (cfg_aimbot_lock)
            {
                if (circle.type == HitObjectType::Slider)
                {
                    uintptr_t osu_manager = *(uintptr_t *)(osu_manager_ptr);
                    uintptr_t hit_manager_ptr = *(uintptr_t *)(osu_manager + 0x48);
                    uintptr_t hit_objects_list_ptr = *(uintptr_t *)(hit_manager_ptr + 0x48);
                    uintptr_t hit_objects_list_items_ptr = *(uintptr_t *)(hit_objects_list_ptr + 0x4);
                    uintptr_t hit_object_ptr = *(uintptr_t *)(hit_objects_list_items_ptr + 0x8 + 0x4 * current_beatmap.hit_object_idx);
                    uintptr_t animation_ptr = *(uintptr_t *)(hit_object_ptr + 0xA4);
                    float slider_ball_x = *(float *)(animation_ptr + 0x4C);
                    float slider_ball_y = *(float *)(animation_ptr + 0x50);
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
            if (cfg_relax_lock && !circle.clicked)
            {
                if (cfg_relax_style == 'a')
                    current_click = current_click == left_click[0] ? right_click[0] : left_click[0];
                send_keyboard_input(current_click, 0);
                FR_INFO_FMT("hit %d!, %d %d", current_beatmap.hit_object_idx, circle.start_time, circle.end_time);
                keyup_delay = circle.end_time ? circle.end_time - circle.start_time : 0.5;
                if (cfg_relax_checks_od)
                {
                    float random_delay = rand_range_f(.1337f, current_beatmap.od_window - .5f);
                    if ((keyup_delay + random_delay) < (circle.end_time - circle.start_time))
                        keyup_delay += random_delay;
                }
                if (cfg_timewarp_enabled)
                {
                    double timewarp_playback_rate_div_100 = cfg_timewarp_playback_rate / 100.0;
                    keyup_delay /= timewarp_playback_rate_div_100;
                }
                else if (circle.type == HitObjectType::Slider || circle.type == HitObjectType::Spinner)
                {
                    if (current_beatmap.mods & Mods::DoubleTime)
                        keyup_delay /= 1.5;
                    else if (current_beatmap.mods & Mods::HalfTime)
                        keyup_delay /= 0.75;
                }
                keydown_time = ImGui::GetTime();
                circle.clicked = true;
                act_before_ms = .0f;
            }
        }
        if (audio_time >= circle.end_time)
        {
            current_beatmap.hit_object_idx++;
            if (current_beatmap.hit_object_idx >= current_beatmap.hit_objects.size())
            {
                current_beatmap.ready = false;
            }
            else if (cfg_aimbot_lock)
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
    }
    if (cfg_relax_lock && keydown_time && ((ImGui::GetTime() - keydown_time) * 1000.0 > keyup_delay))
    {
        keydown_time = 0.0;
        send_keyboard_input(current_click, KEYEVENTF_KEYUP);
    }
}
