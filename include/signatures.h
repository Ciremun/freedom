#pragma once

#include <stdint.h>

#include "pattern.h"

constexpr auto approach_rate_signature              { pattern::build<"8B 85 B0 FE FF FF D9 58 2C"> };
constexpr auto approach_rate_signature_2            { pattern::build<"8B 85 B0 FE FF FF D9 40 38 D9 58 2C"> };
constexpr auto circle_size_signature                { pattern::build<"8B 85 B0 FE FF FF D9 58 30"> };
constexpr auto overall_difficulty_signature         { pattern::build<"8B 85 B0 FE FF FF D9 58 38"> };
constexpr auto beatmap_onload_signature             { pattern::build<"8B 86 80 00 00 00"> };
constexpr auto current_scene_signature              { pattern::build<"A1....A3....A1....A3"> };
constexpr auto selected_song_signature              { pattern::build<"D9 EE DD 5C 24 10 83 3D"> };
constexpr auto audio_time_signature                 { pattern::build<"F7 DA 3B C2"> };
constexpr auto osu_manager_signature                { pattern::build<"85 C9"> };
constexpr auto binding_manager_signature            { pattern::build<"8D 45 D8 50 8B 0D"> };
constexpr auto selected_replay_signature            { pattern::build<"8B 46 38 83 78 30 00"> };
constexpr auto osu_username_signature               { pattern::build<"8B 01 8B 40 28 FF 50 18 8B 15"> };
constexpr auto window_manager_signature             { pattern::build<"83 C2 04 8B 0D"> };
constexpr auto score_multiplier_signature           { pattern::build<"8B F1 D9 E8 83 FA 04 0F 83"> };
constexpr auto update_timing_signature              { pattern::build<"D9 C0 DD 05"> };
constexpr auto update_timing_signature_2            { pattern::build<"DE E9 DD 1D"> };
constexpr auto check_timewarp_signature             { pattern::build<"D9 E8 DE F1 DE C9"> };

constexpr auto parse_beatmap_function_signature     { pattern::build<"55 8B EC 57 56 53 81 EC 58 01 00 00 8B F1 8D BD B8 FE FF FF B9 4E 00 00 00 33 C0 F3 AB 8B CE 89 8D B0 FE FF FF"> };
constexpr auto current_scene_function_signature     { pattern::build<"55 8B EC 57 56 53 50 8B D9 83 3D"> };
constexpr auto beatmap_onload_function_signature    { pattern::build<"55 8B EC 57 56 53 83 EC 44 8B F1 B9"> };
constexpr auto selected_song_function_signature     { pattern::build<"55 8B EC 83 E4 F8 57 56 83 EC 38 83 3D"> };
constexpr auto audio_time_function_signature        { pattern::build<"55 8B EC 83 E4 F8 57 56 83 EC 38 83 3D"> };
constexpr auto osu_manager_function_signature       { pattern::build<"55 8B EC 57 56 53 83 EC 14 80 3D"> };
constexpr auto binding_manager_function_signature   { pattern::build<"55 8B EC 57 56 83 EC 58 8B F1 8D 7D A0"> };
constexpr auto selected_replay_function_signature   { pattern::build<"55 8B EC 57 56 53 81 EC A0 00 00 00 8B F1 8D BD 68 FF FF FF B9 22 00 00 00 33 C0 F3 AB 8B CE 8B F1 8D 7D E0"> };
constexpr auto window_manager_function_signature    { pattern::build<"57 56 53 83 EC 6C 8B F1 8D 7D A8 B9 12 00 00 00 33 C0 F3 AB 8B CE 89 4D 94"> };
constexpr auto update_timing_function_signature     { pattern::build<"55 8B EC 83 E4 F8 57 56 83 EC 18 8B F9 8B 0D"> };
constexpr auto check_timewarp_function_signature    { pattern::build<"55 8B EC 57 56 53 81 EC B0 01 00 00 8B F1 8D BD 50 FE FF FF B9 68 00 00 00 33 C0"> };
constexpr auto osu_client_id_function_signature     { pattern::build<"8B F1 8D 7D C4 B9 0C 00 00 00 33 C0 F3 AB 8B CE 89 4D C0 8B 15"> };
constexpr auto username_function_signature          { pattern::build<"55 8B EC 57 56 53 83 EC 08 33 C0 89 45 EC 89 45 F0 8B F2 8B CE 8B 01 8B 40 30"> };
constexpr auto update_flashlight_function_signature { pattern::build<"55 8B EC 56 83 EC 14 8B F1 8B 56 5C"> };
constexpr auto check_flashlight_function_signature  { pattern::build<"55 8B EC 57 56 53 83 EC 18 8B F9 80"> };
