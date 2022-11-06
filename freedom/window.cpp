#include "mem.h"
#include "vector.h"

Vector2<float> window_size(.0f, .0f);
Vector2<float> playfield_size(.0f, .0f);
Vector2<float> playfield_position(.0f, .0f);

float window_ratio = .0f;
float playfield_ratio = .0f;

void calc_playfield_manual(float window_x, float window_y)
{
    window_size.x = window_x;
    window_size.y = window_y;
    window_ratio = window_size.y / 480.0f;
    playfield_size.x = 512.0f * window_ratio;
    playfield_size.y = 384.0f * window_ratio;
    playfield_position.x = (window_size.x - playfield_size.x) / 2.0f;
    playfield_position.y =  (window_size.y - playfield_size.y) / 4.0f * 3.0f + (-16.0f * window_ratio);
    playfield_ratio = playfield_size.y / 384.0f;
    FR_INFO_FMT("calc_playfield_manual: window_x: %f, window_y: %f", window_x, window_y);
}

bool calc_playfield_from_window(uintptr_t window_manager_ptr)
{
    if (!window_manager_ptr)
        return false;

    uintptr_t window_manager = *(uintptr_t *)window_manager_ptr;
    if (!window_manager)
        return false;

    uintptr_t window_size_ptr = *(uintptr_t *)(window_manager + 0x4);
    window_size.x = (float)(*(uint32_t *)(window_size_ptr + 0x4));
    window_size.y = (float)(*(uint32_t *)(window_size_ptr + 0x8));
    playfield_size.x = *(float *)(window_manager + 0x8);
    playfield_size.y = *(float *)(window_manager + 0xC);
    playfield_position.x = *(float *)(window_manager + 0x18);
    playfield_position.y = *(float *)(window_manager + 0x1C);
    playfield_ratio = playfield_size.y / 384.0f;
    FR_INFO_FMT("calc_playfield_from_window: window_x: %f, window_y: %f", window_size.x, window_size.y);
    return true;
}
