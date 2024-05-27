#pragma once

#include "ui/config.h"

extern float base_stddev;
extern float od_window;
extern float od_window_left_offset;
extern float od_window_right_offset;
extern float od_check_ms;

extern int wait_hitobjects_min;
extern int wait_hitobjects_max;

void relax_on_beatmap_load();
void update_relax(Circle &circle, const int32_t audio_time);

void display_keypress_info();
