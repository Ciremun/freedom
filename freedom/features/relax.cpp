#include "features/relax.h"
#include "window.h"
#include <cmath>

float od_window = 5.f;
float od_window_left_offset = .0f;
float od_window_right_offset = .0f;
float od_check_ms = .0f;

float jumping_window_offset = .0f;

int wait_hitobjects_min = 2;
int wait_hitobjects_max = 5;

static char current_click = cfg_relax_style == 'a' ? right_click[0] : left_click[0];

float gaussian_rand()
{
    static bool has_spare = false;
    static double spare;

    if (has_spare)
    {
        has_spare = false;
        return spare;
    }

    has_spare = true;
    static double u, v, s;
    do
    {
        u = (rand() / ((double)RAND_MAX)) * 2.0 - 1.0;
        v = (rand() / ((double)RAND_MAX)) * 2.0 - 1.0;
        s = u * u + v * v;
    } while (s >= 1.0 || s == 0.0);

    s = sqrt(-2.0 * log(s) / s);
    spare = v * s;
    return u * s;
}

float sharp_gaussian_rand(float mean, float stddev)
{
    // Increase the range the relax bot clicks in by adjusting the standard deviation here
    float adjusted_stddev = stddev * 3.1f; // Increase range (less = less min max ms delay and more = yea..more min max delay. 2.0-4.4 recommended depending on beatmap)
    return mean + gaussian_rand() * adjusted_stddev * 0.390f; // Maintain sharpness
}

void calc_od_timing()
{
    const auto sharp_gaussian_rand_range_f = [](float mean, float stddev) -> float
        {
            return sharp_gaussian_rand(mean, stddev);
        };
    const auto rand_range_i = [](int i_min, int i_max) -> int
        {
            return rand() % (i_max + 1 - i_min) + i_min;
        };
    if (cfg_relax_checks_od && (od_check_ms == .0f))
    {
        od_check_ms = sharp_gaussian_rand_range_f((od_window_left_offset + od_window_right_offset) / 2.0, (od_window_right_offset - od_window_left_offset) / 4.0); // Adjusted stddev
        if (cfg_jumping_window)
        {
            static uint32_t hit_objects_passed = current_beatmap.hit_object_idx;
            static int wait_hitobjects_count = rand_range_i(wait_hitobjects_min, wait_hitobjects_max);
            if (current_beatmap.hit_object_idx - hit_objects_passed >= wait_hitobjects_count)
            {
                if (rand_range_i(0, 1) >= 1)
                    jumping_window_offset = sharp_gaussian_rand_range_f((od_window + .1337f - od_window_left_offset) / 2.0, (od_window - od_window_left_offset) / 4.0); // Adjusted stddev
                else
                    jumping_window_offset = -sharp_gaussian_rand_range_f((od_window_right_offset + .1337f) / 2.0, (od_window_right_offset) / 4.0); // Adjusted stddev
                hit_objects_passed = current_beatmap.hit_object_idx;
                wait_hitobjects_count = rand_range_i(wait_hitobjects_min, wait_hitobjects_max);
            }
            od_check_ms += jumping_window_offset;
        }
    }
}

Vector2<float> mouse_position()
{
    Vector2<float> mouse_pos;
    uintptr_t osu_manager = *(uintptr_t*)(osu_manager_ptr);
    uintptr_t osu_ruleset_ptr = *(uintptr_t*)(osu_manager + OSU_MANAGER_RULESET_PTR_OFFSET);
    mouse_pos.x = *(float*)(osu_ruleset_ptr + OSU_RULESET_MOUSE_X_OFFSET);
    mouse_pos.y = *(float*)(osu_ruleset_ptr + OSU_RULESET_MOUSE_Y_OFFSET);

    return mouse_pos;
}

void update_relax(Circle& circle, const int32_t audio_time)
{
    static double keydown_time = 0.0;
    static double keyup_delay = 0.0;
    static double last_key_action_time = 0.0; // Track the time of the last key action

    if (cfg_relax_lock)
    {
        calc_od_timing();

        auto current_time = audio_time + od_check_ms;
        auto valid_timing = current_time >= circle.start_time;
        auto mouse_pos = mouse_position();
        Vector2 screen_pos = playfield_to_screen(circle.position);
        auto scalar_dist = sqrt((mouse_pos.x - screen_pos.x) * (mouse_pos.x - screen_pos.x) + (mouse_pos.y - screen_pos.y) * (mouse_pos.y - screen_pos.y));
        // auto valid_position = scalar_dist <= current_beatmap.scaled_hit_object_radius;

        if (valid_timing)
        {
            if (!circle.clicked)
            {
                // Only alternates clicks if the time since the last key action is less than 800ms (hence the 0.8)
                if (cfg_relax_style == 'a' && (ImGui::GetTime() - last_key_action_time) < 0.8)
                {
                    current_click = current_click == left_click[0] ? right_click[0] : left_click[0];
                }

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
                last_key_action_time = ImGui::GetTime(); // Update the last key action time
                circle.clicked = true;
                od_check_ms = .0f;
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
