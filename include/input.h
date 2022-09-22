#pragma once

#include <windows.h>

#include "window.h"
#include "freedom.h"
#include "detours.h"

extern char left_click[2];
extern char right_click[2];

void init_input();
void send_keyboard_input(char wVk, DWORD dwFlags);
void move_mouse_to(int x, int y);
