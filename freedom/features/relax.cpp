#include "features/relax.h"
#include "window.h"
#include <cmath>
#include <chrono>
#include <deque>
#include <imgui.h> // Include ImGui header for rendering

float od_window = 5.f;
float od_window_left_offset = .0f;
float od_window_right_offset = .0f;
float od_check_ms = .0f;
float jumping_window_offset = .0f;
int wait_hitobjects_min = 2;
int wait_hitobjects_max = 5;

static char current_click = cfg_relax_style == 'a' ? right_click[0] : left_click[0];
static float range_shift = 0.0f;
static std::chrono::time_point<std::chrono::steady_clock> last_shift_time = std::chrono::steady_clock::now();
static int click_count = 0;
static std::deque<std::chrono::time_point<std::chrono::steady_clock>> key_press_times;

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
    mean += range_shift;
    float adjusted_stddev = stddev * 3.3f;

    auto now = std::chrono::steady_clock::now();
    while (!key_press_times.empty() && std::chrono::duration_cast<std::chrono::milliseconds>(now - key_press_times.front()).count() >= 500)
    {
        key_press_times.pop_front();
    }

    if (key_press_times.size() > 4) // Also change line 220
    {
        adjusted_stddev *= 1.5f;
    }

    return mean + gaussian_rand() * adjusted_stddev * 0.38f;
}

void update_range_shift()
{
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last_shift_time).count();
    if (duration >= 9)
    {
        range_shift = (rand() / (float)RAND_MAX) * 28.0f - 14.0f;
        last_shift_time = now;
    }
}

bool should_add_extremity()
{
    static int last_extremity_click = 0;
    if (click_count - last_extremity_click >= (rand() % 41 + 15))
    {
        last_extremity_click = click_count;
        return true;
    }
    return false;
}

float add_extremity(float timing)
{
    float base_extremity = 2.1f;
    float random_offset = ((rand() / (float)RAND_MAX) * 0.8f) - 0.6f;
    float extremity = 0.5f * od_window * (base_extremity + random_offset);
    return (rand() % 2 == 0) ? timing + extremity : timing - extremity;
}

void calc_od_timing()
{
    update_range_shift();

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
        od_check_ms = sharp_gaussian_rand_range_f((od_window_left_offset + od_window_right_offset) / 2.0, (od_window_right_offset - od_window_left_offset) / 4.0);
        if (should_add_extremity())
        {
            od_check_ms = add_extremity(od_check_ms);
        }
        if (cfg_jumping_window)
        {
            static uint32_t hit_objects_passed = current_beatmap.hit_object_idx;
            static int wait_hitobjects_count = rand_range_i(wait_hitobjects_min, wait_hitobjects_max);
            if (current_beatmap.hit_object_idx - hit_objects_passed >= wait_hitobjects_count)
            {
                if (rand_range_i(0, 1) >= 1)
                    jumping_window_offset = sharp_gaussian_rand_range_f((od_window + .1337f - od_window_left_offset) / 2.0, (od_window - od_window_left_offset) / 4.0);
                else
                    jumping_window_offset = -sharp_gaussian_rand_range_f((od_window_right_offset + .1337f) / 2.0, (od_window_right_offset) / 4.0);
                hit_objects_passed = current_beatmap.hit_object_idx;
                wait_hitobjects_count = rand_range_i(wait_hitobjects_min, wait_hitobjects_max);
            }
            od_check_ms += jumping_window_offset;
        }
        click_count++;
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
    static double last_key_action_time = 0.0;
    static bool first_click = true;

    if (cfg_relax_lock)
    {
        calc_od_timing();

        auto current_time = audio_time + od_check_ms;
        auto valid_timing = current_time >= circle.start_time;
        auto mouse_pos = mouse_position();
        Vector2 screen_pos = playfield_to_screen(circle.position);
        auto scalar_dist = sqrt((mouse_pos.x - screen_pos.x) * (mouse_pos.x - screen_pos.x) + (mouse_pos.y - screen_pos.y) * (mouse_pos.y - screen_pos.y));

        if (valid_timing)
        {
            if (!circle.clicked)
            {
                if (first_click || (ImGui::GetTime() - last_key_action_time) >= 0.3)
                {
                    current_click = right_click[0];
                    first_click = false;
                }
                else if (cfg_relax_style == 'a')
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
                else if (circle.type == HitObjectType::Slider or circle.type == HitObjectType::Spinner)
                {
                    if (current_beatmap.mods & Mods::DoubleTime)
                        keyup_delay /= 1.5;
                    else if (current_beatmap.mods & Mods::HalfTime)
                        keyup_delay /= 0.75;
                }
                keydown_time = ImGui::GetTime();
                last_key_action_time = ImGui::GetTime();
                circle.clicked = true;
                od_check_ms = .0f;

                key_press_times.push_back(std::chrono::steady_clock::now());
            }
        }
    }
    if (cfg_relax_lock && keydown_time && ((ImGui::GetTime() - keydown_time) * 1000.0 > keyup_delay))
    {
        keydown_time = 0.0;
        send_keyboard_input(current_click, KEYEVENTF_KEYUP);
    }
}

void display_keypress_info()
{
    static bool position_initialized = false;
    auto now = std::chrono::steady_clock::now();
    while (!key_press_times.empty() && std::chrono::duration_cast<std::chrono::seconds>(now - key_press_times.front()).count() >= 1)
    {
        key_press_times.pop_front();
    }

    if (!position_initialized)
    {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 300, 80), ImGuiCond_FirstUseEver); // Set initial position only once
        position_initialized = true;
    }

    ImGui::Begin("Key Press Info", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

    ImVec4 color = (key_press_times.size() > 4) ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1);
    ImGui::TextColored(color, "Key Presses: %d", static_cast<int>(key_press_times.size()));

    ImGui::End();
}


void relax_on_beatmap_load()
{
    if (cfg_relax_lock)
    {
        current_click = cfg_relax_style == 'a' ? right_click[0] : left_click[0];
    }
}
