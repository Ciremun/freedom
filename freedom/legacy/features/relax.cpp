#include "legacy/features/relax.h"
#include "legacy/window.h"

static char current_click = cfg_relax_style == 'a' ? right_click[0] : left_click[0];

void update_relax(Circle &circle, const int32_t audio_time)
{
    static double keydown_time = 0.0;
    static double keyup_delay = 0.0;

    if (cfg_relax_lock)
    {
        auto current_time = audio_time;
        auto valid_timing = current_time >= circle.start_time;
        // auto mouse_pos = mouse_position();
        // Vector2 screen_pos = playfield_to_screen(circle.position);
        // auto scalar_dist = sqrt((mouse_pos.x - screen_pos.x) * (mouse_pos.x - screen_pos.x) + (mouse_pos.y - screen_pos.y) * (mouse_pos.y - screen_pos.y));
        // auto valid_position = scalar_dist <= current_beatmap.scaled_hit_object_radius;

        if (valid_timing)
        {
            if (!circle.clicked)
            {
                if (cfg_relax_style == 'a')
                    current_click = current_click == left_click[0] ? right_click[0] : left_click[0];

                send_keyboard_input(current_click, 0);
                FR_INFO("Relax hit %d!, %d %d", current_beatmap.hit_object_idx, circle.start_time, circle.end_time);
                keyup_delay = circle.end_time ? circle.end_time - circle.start_time : 0.5;

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
            }
        }
    }
    if (cfg_relax_lock && keydown_time && ((ImGui::GetTime() - keydown_time) * 1000.0 > keyup_delay))
    {
        keydown_time = 0.0;
        send_keyboard_input(current_click, KEYEVENTF_KEYUP);
    }
}

void relax_on_beatmap_load()
{
    if (cfg_relax_lock)
    {
        current_click = cfg_relax_style == 'a' ? right_click[0] : left_click[0];
    }
}
