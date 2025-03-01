#pragma once

#include <windows.h>

#include "legacy/window.h"
#include "freedom.h"
#include "legacy/scan.h"
#include "legacy/vector.h"

extern uintptr_t binding_manager_code_start;
extern uintptr_t binding_manager_ptr;

extern char left_click[2];
extern char right_click[2];

void init_input();
void send_keyboard_input(char wVk, DWORD dwFlags);
Vector2<float> mouse_position();
void move_mouse_to(int x, int y);
