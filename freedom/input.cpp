#include "input.h"

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
    inputs[0].mi.dx = (x * (0xFFFF / 1920));
    inputs[0].mi.dy = (y * (0xFFFF / 1080));
    inputs[0].mi.mouseData = 0;
    inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
    inputs[0].mi.time = 0;
    inputs[0].mi.dwExtraInfo = 0;
    SendInput(1, inputs, sizeof(INPUT));
}
