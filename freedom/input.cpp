#include "input.h"

uintptr_t binding_manager_code_start = 0;
uintptr_t binding_manager_ptr = 0;

char left_click[2] = { 'Z', '\0' };
char right_click[2] = { 'X', '\0' };

void init_input()
{
    if (binding_manager_ptr)
    {
        char sus_left_click = '\0';
        if (internal_memory_read(g_process, binding_manager_ptr, &sus_left_click))
        {
            char sus_right_click = '\0';
            if (internal_memory_read(g_process, binding_manager_ptr + 0x10, &sus_right_click))
            {
                if (('A' <= sus_left_click && sus_left_click <= 'Z') && ('A' <= sus_right_click && sus_right_click <= 'Z'))
                {
                    left_click[0] = sus_left_click;
                    right_click[0] = sus_right_click;
                }
            }
        }
    }
    FR_INFO("Left Click: %c", left_click[0]);
    FR_INFO("Right Click: %c", right_click[0]);

    primary_monitor.x = (float)GetSystemMetrics(SM_CXSCREEN);
    primary_monitor.y = (float)GetSystemMetrics(SM_CYSCREEN);
    if (!calc_playfield_from_window())
        calc_playfield_manual(primary_monitor.x, primary_monitor.y);
}

void send_keyboard_input(char wVk, DWORD dwFlags)
{
    INPUT inputs[1];
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = 0;
    inputs[0].ki.wScan = 0;
    inputs[0].ki.time = 0;
    inputs[0].ki.dwExtraInfo = 0;
    inputs[0].ki.dwFlags = dwFlags;
    inputs[0].ki.wVk = wVk;
    SendInput(1, inputs, sizeof(INPUT));
}

Vector2<float> mouse_position()
{
    Vector2<float> mouse_pos(.0f, .0f);
    uintptr_t osu_manager = *(uintptr_t*)(osu_manager_ptr);
    if (!osu_manager) return mouse_pos;
    uintptr_t osu_ruleset_ptr = *(uintptr_t*)(osu_manager + OSU_MANAGER_RULESET_PTR_OFFSET);
    if (!osu_ruleset_ptr) return mouse_pos;
    mouse_pos.x = *(float*)(osu_ruleset_ptr + OSU_RULESET_MOUSE_X_OFFSET);
    mouse_pos.y = *(float*)(osu_ruleset_ptr + OSU_RULESET_MOUSE_Y_OFFSET);
    return mouse_pos;
}

void move_mouse_to(int x, int y)
{
    x += client_offset.x;
    y += client_offset.y;
    INPUT inputs[1];
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dx = (x * (0xFFFF / primary_monitor.x));
    inputs[0].mi.dy = (y * (0xFFFF / primary_monitor.y));
    inputs[0].mi.mouseData = 0;
    inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
    inputs[0].mi.time = 0;
    inputs[0].mi.dwExtraInfo = 0;
    SendInput(1, inputs, sizeof(INPUT));
}
