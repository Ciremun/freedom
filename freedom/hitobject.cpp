#define LZMA_IMPLEMENTATION
#include "lzma.h"

#include "hitobject.h"

BeatmapData current_beatmap;
ReplayData current_replay;
Scene current_scene = Scene::MAIN_MENU;

bool start_parse_beatmap = false;
bool start_parse_replay = false;
bool target_first_circle = true;

static inline bool is_playing(uintptr_t audio_time_ptr)
{
    return *(bool *)(audio_time_ptr + 0x30);
}

void process_hitobject()
{
    if (start_parse_beatmap)
    {
        parse_beatmap(osu_manager_ptr, current_beatmap);
        target_first_circle = true;
        start_parse_beatmap = false;
    }

    if (start_parse_replay)
    {
        current_replay.clear();
        selected_replay_ptr += 0x30;
        uintptr_t selected_replay = *(uintptr_t *)selected_replay_ptr;
        size_t compressed_data_size = *(uint32_t *)(selected_replay + 0x4);
        FR_INFO_FMT("compressed_data_size: %zu", compressed_data_size);
        uint8_t *compressed_data = (uint8_t *)(selected_replay + 0x8);
        size_t replay_data_size = *(size_t *)&compressed_data[LZMA_HEADER_SIZE - 8];
        FR_INFO_FMT("replay_data_size: %zu", replay_data_size);
        static std::vector<uint8_t> replay_data;
        replay_data.reserve(replay_data_size);
        lzma_uncompress(&replay_data[0], &replay_data_size, compressed_data, &compressed_data_size);
        const char *replay_data_ptr = (const char *)&replay_data[0];
        size_t next_comma_position = 0;
        ReplayEntryData entry;
        while (entry.ms_since_last_frame != -12345)
        {
            if (sscanf(replay_data_ptr, "%lld|%f|%f|%d", &entry.ms_since_last_frame, &entry.position.x, &entry.position.y, &entry.keypresses) == 4)
            {
                entry.position = playfield_to_screen(entry.position);
                current_replay.entries.push_back(entry); // fixme - reserve
            }
            else
                break;
            while (next_comma_position < replay_data_size)
                if (replay_data[++next_comma_position] == ',')
                    break;
            if (next_comma_position >= replay_data_size)
                break;
            replay_data_ptr += (const char *)&replay_data[next_comma_position] - replay_data_ptr + 1;
        }
        FR_INFO_FMT("current_replay.size: %zu", current_replay.entries.size());
        start_parse_replay = false;
    }

    if (current_scene == Scene::GAME && is_playing(audio_time_ptr))
    {
        int32_t audio_time = *(int32_t *)audio_time_ptr;
        ReplayEntryData &entry = current_replay.current_entry();
        if (audio_time >= current_replay.replay_ms + entry.ms_since_last_frame)
        {
            static bool left = false;
            static bool right = false;
            if (current_replay.entries_idx < current_replay.entries.size())
            {
                move_mouse_to(entry.position.x, entry.position.y);

                // fixme refactor
                if (entry.keypresses != 0)
                {
                    if (entry.keypresses == 15)
                    {
                        if (!left)
                        {
                            send_keyboard_input(left_click[0], 0);
                            left = true;
                        }
                        if (!right)
                        {
                            send_keyboard_input(right_click[0], 0);
                            right = true;
                        }
                    }
                    if (entry.keypresses == 10)
                    {
                        if (left)
                        {
                            send_keyboard_input(left_click[0], KEYEVENTF_KEYUP);
                            left = false;
                        }
                        if (!right)
                        {
                            send_keyboard_input(right_click[0], 0);
                            right = true;
                        }
                    }
                    if (entry.keypresses == 5)
                    {
                        if (!left)
                        {
                            send_keyboard_input(left_click[0], 0);
                            left = true;
                        }
                        if (right)
                        {
                            send_keyboard_input(right_click[0], KEYEVENTF_KEYUP);
                            right = false;
                        }
                    }
                }
                else
                {
                    if (left)
                    {
                        send_keyboard_input(left_click[0], KEYEVENTF_KEYUP);
                        left = false;
                    }
                    if (right)
                    {
                        send_keyboard_input(right_click[0], KEYEVENTF_KEYUP);
                        right = false;
                    }
                }

                current_replay.replay_ms += entry.ms_since_last_frame;
                current_replay.entries_idx++;
            }
            else
            {
                if (left)
                {
                    send_keyboard_input(left_click[0], KEYEVENTF_KEYUP);
                    left = false;
                }
                if (right)
                {
                    send_keyboard_input(right_click[0], KEYEVENTF_KEYUP);
                    right = false;
                }
            }
        }
    }

    static double keydown_time = 0.0;
    static double keyup_delay = 0.0;
    static float fraction_of_the_distance = 0.0f;
    static Vector2 direction(0.0f, 0.0f);
    static Vector2 mouse_position(0.0f, 0.0f);
    if ((cfg_relax_lock || cfg_aimbot_lock) && current_scene == Scene::GAME && current_beatmap.ready && is_playing(audio_time_ptr))
    {
        int32_t audio_time = *(int32_t *)audio_time_ptr;
        Circle* circle = current_beatmap.current_circle();
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
                direction = prepare_hitcircle_target(osu_manager_ptr, circle->position, mouse_position);
                fraction_of_the_distance = cfg_fraction_modifier;
                target_first_circle = false;
            }
        }
        if (audio_time >= circle->start_time)
        {
            if (cfg_aimbot_lock)
            {
                if (circle->type == HitObjectType::Slider)
                {
                    Slider *slider = (Slider *)circle;
                    if (slider->curves.size() == 2)
                    {
                        direction = prepare_hitcircle_target(osu_manager_ptr, slider->curves[1], mouse_position);
                        fraction_of_the_distance = cfg_fraction_modifier;
                    }
                    else
                    {
                        static int32_t prev_audio_time = audio_time;
                        int32_t circle_time = slider->end_time - slider->start_time;
                        if ((audio_time - prev_audio_time) >= (circle_time / slider->curves.size()))
                        {
                            if (slider->curve_idx < slider->curves.size())
                            {
                                direction = prepare_hitcircle_target(osu_manager_ptr, slider->curves[slider->curve_idx++], mouse_position);
                                fraction_of_the_distance = cfg_fraction_modifier;
                                prev_audio_time = audio_time;
                            }
                        }
                    }
                }
                if (circle->type == HitObjectType::Spinner)
                {
                    auto& center = circle->position;
                    static const float radius = 60.0f;
                    static const float PI = 3.14159f;
                    static float angle = .0f;
                    Vector2 next_point_on_circle(center.x + radius * cosf(angle), center.y + radius * sinf(angle));
                    direction = prepare_hitcircle_target(osu_manager_ptr, next_point_on_circle, mouse_position);
                    fraction_of_the_distance = 1.0f;
                    angle > 2 * PI ? angle = 0 : angle += cfg_spins_per_minute / (3 * PI) * ImGui::GetIO().DeltaTime;
                }
            }
            if (cfg_relax_lock && !circle->clicked)
            {
                send_keyboard_input(left_click[0], 0);
                FR_INFO_FMT("hit %d!, %d %d", current_beatmap.hit_object_idx, circle->start_time, circle->end_time);
                keyup_delay = circle->end_time ? circle->end_time - circle->start_time : 0.5;
                if (circle->type == HitObjectType::Slider || circle->type == HitObjectType::Spinner)
                {
                    if (current_beatmap.mods & Mods::DoubleTime)
                        keyup_delay /= 1.5;
                    else if (current_beatmap.mods & Mods::HalfTime)
                        keyup_delay /= 0.75;
                }
                keydown_time = ImGui::GetTime();
                circle->clicked = true;
            }
        }
        if (audio_time >= circle->end_time)
        {
            current_beatmap.hit_object_idx++;
            if (current_beatmap.hit_object_idx >= current_beatmap.hit_objects.size())
            {
                current_beatmap.ready = false;
            }
            else if (cfg_aimbot_lock)
            {
                Circle* next_circle = current_beatmap.current_circle();
                switch (next_circle->type)
                {
                    case HitObjectType::Circle:
                    case HitObjectType::Slider:
                    case HitObjectType::Spinner:
                    {
                        direction = prepare_hitcircle_target(osu_manager_ptr, next_circle->position, mouse_position);
                        fraction_of_the_distance = cfg_fraction_modifier;
                    } break;
                }
            }
        }
    }
    if (cfg_relax_lock && keydown_time && ((ImGui::GetTime() - keydown_time) * 1000.0 > keyup_delay))
    {
        keydown_time = 0.0;
        send_keyboard_input(left_click[0], KEYEVENTF_KEYUP);
    }
}
