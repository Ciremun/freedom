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
    FR_INFO_FMT("Left Click: %c", left_click[0]);
    FR_INFO_FMT("Right Click: %c", right_click[0]);

    if (!calc_playfield_from_window())
        calc_playfield_manual(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
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

void move_mouse_to(int x, int y)
{
    INPUT inputs[1];
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dx = (x * (0xFFFF / window_size.x));
    inputs[0].mi.dy = (y * (0xFFFF / window_size.y));
    inputs[0].mi.mouseData = 0;
    inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
    inputs[0].mi.time = 0;
    inputs[0].mi.dwExtraInfo = 0;
    SendInput(1, inputs, sizeof(INPUT));
}
