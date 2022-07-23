#include "utility.h"
#include "vector.h"

Vector2 window_size(1920.0f, 1080.0f);
float window_ratio = window_size.y / 480.0f;
Vector2 playfield_size(512.0f * window_ratio, 384.0f * window_ratio);
Vector2 playfield_position((window_size.x - playfield_size.x) / 2.0f, (window_size.y - playfield_size.y) / 4.0f * 3.0f + (-16.0f * window_ratio));
float playfield_ratio = playfield_size.y / 384.0f;

void calc_playfield(float window_x, float window_y)
{
    FR_INFO_FMT("window_x: %f, window_y: %f", window_x, window_y);
    window_size.x = window_x;
    window_size.y = window_y;
    window_ratio = window_size.y / 480.0f;
    playfield_size.x = 512.0f * window_ratio;
    playfield_size.y = 384.0f * window_ratio;
    playfield_position.x = (window_size.x - playfield_size.x) / 2.0f;
    playfield_position.y =  (window_size.y - playfield_size.y) / 4.0f * 3.0f + (-16.0f * window_ratio);
    playfield_ratio = playfield_size.y / 384.0f;
}
