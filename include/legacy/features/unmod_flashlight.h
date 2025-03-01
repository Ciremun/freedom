#pragma once

#include <stdint.h>

#include "legacy/hook.h"
#include "legacy/scan.h"
#include "legacy/signatures.h"
#include "legacy/struct_offsets.h"

#include "ui/config.h"

extern uintptr_t update_flashlight_code_start;
extern uint8_t update_flashlight_original_byte;

extern uintptr_t check_flashlight_code_start;
extern uint8_t check_flashlight_original_byte;

void init_unmod_flashlight();

void set_flashlight_alpha_value(float value);
void unmod_flashlight_on_beatmap_load();

void enable_flashlight_hooks();
void disable_flashlight_hooks();
