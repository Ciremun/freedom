#include "input.h"

void send_input(char wVk, DWORD dwFlags)
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
