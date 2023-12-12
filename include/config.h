#pragma once

#include <windows.h>

#include "imgui.h"
#include "imgui_internal.h"

#include "scan.h"
#include "mem.h"

extern int cfg_font_size;
extern int cfg_spins_per_minute;
extern bool cfg_mod_menu_visible;
extern float cfg_fraction_modifier;
extern bool cfg_replay_enabled;
extern bool cfg_replay_aim;
extern bool cfg_replay_keys;
extern bool cfg_replay_hardrock;
extern int cfg_relax_style;
extern bool cfg_relax_lock;
extern bool cfg_aimbot_lock;
extern bool cfg_relax_checks_od;
extern bool cfg_jumping_window;
extern bool cfg_score_multiplier_enabled;
extern float cfg_score_multiplier_value;
extern bool cfg_discord_rich_presence_enabled;
extern bool cfg_flashlight_enabled;
extern bool cfg_timewarp_enabled;
extern double cfg_timewarp_playback_rate;
extern bool cfg_hidden_remover_enabled;

extern std::string cm_load_s;
extern std::string cm_replay_s;
extern std::string cm_score_s;
extern std::string cm_checkflashlight_s;
extern std::string cm_updateflashlight_s;
extern std::string cm_checktime_s;
extern std::string cm_updatevariables_s;

const char *get_imgui_ini_filename(HMODULE hMod);
void set_imgui_ini_handler();
