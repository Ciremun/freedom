#include "hitobject.h"

BeatmapData current_beatmap;
ReplayData current_replay;

bool beatmap_loaded = false;
bool mods_updated = false;
bool start_parse_replay = false;

bool scene_is_game(Scene *current_scene_ptr)
{
    if (current_scene_ptr == 0) return false;
    return *current_scene_ptr == Scene::GAME;
}

bool is_playing(uintptr_t audio_time_ptr)
{
    if (audio_time_ptr == 0) return false;
    return *(bool *)(audio_time_ptr + OSU_AUDIO_TIME_IS_PLAYING_OFFSET);
}

bool is_replay_mode(uintptr_t osu_manager_ptr)
{
    if (osu_manager_ptr == 0) return false;
    uintptr_t osu_manager = *(uintptr_t *)(osu_manager_ptr);
    if (osu_manager == 0) return false;
    return *(bool *)(osu_manager + OSU_MANAGER_IS_REPLAY_MODE_OFFSET);
}

void process_hitobject()
{
    if (beatmap_loaded)
    {
        if (parse_beatmap(osu_manager_ptr, current_beatmap))
        {
            relax_on_beatmap_load();
            aimbot_on_beatmap_load();
        }
        replay_on_beatmap_load();
        unmod_flashlight_on_beatmap_load();
        unmod_hidden_on_beatmap_load();
        beatmap_loaded = false;
    }

    if (mods_updated)
    {
        char selected_mods[64] = "Unknown";
        FR_INFO("mods updated: %s", selected_mods_ptr ? mods_to_string(*selected_mods_ptr, selected_mods) : "Unknown");
        ar_parameter.apply_mods();
        mods_updated = false;
    }

    if (start_parse_replay)
    {
        std::thread(parse_replay, selected_replay_ptr, std::ref(current_replay)).detach();
        start_parse_replay = false;
    }

    // NOTE(Ciremun): Hack. Make a hook to improve performance
    if (selected_song_ptr && cfg_relax_lock && cfg_relax_checks_od)
    {
        uintptr_t selected_song = 0;
        if (internal_memory_read(g_process, selected_song_ptr, &selected_song))
        {
            uintptr_t song_str_ptr = selected_song + OSU_BEATMAP_SONG_STR_OFFSET;
            static uintptr_t prev_song_str_ptr = 0;
            if (song_str_ptr != prev_song_str_ptr)
            {
                float od = .0f;
                if (internal_memory_read(g_process, selected_song + od_parameter.offset, &od))
                {
                    // TODO(Ciremun): check mods here
                    od_window = 80.f - 6.f * od;
                    od_window -= .5f;
                }
            }
            prev_song_str_ptr = song_str_ptr;
        }
    }

    if ((cfg_relax_lock || cfg_aimbot_lock || cfg_replay_enabled) && scene_is_game(current_scene_ptr) && is_playing(audio_time_ptr) && !is_replay_mode(osu_manager_ptr))
    {
        if (cfg_replay_enabled && current_replay.ready)
            update_replay();

        if ((cfg_relax_lock || cfg_aimbot_lock) && current_beatmap.ready)
        {
            Circle& circle = current_beatmap.current_circle();
            int32_t audio_time = *(int32_t *)audio_time_ptr;

            update_aimbot(circle, audio_time);
            update_relax(circle, audio_time);

            // NOTE(Ciremun): Advance HitObject Index
            if (audio_time + od_check_ms >= circle.end_time)
            {
                current_beatmap.hit_object_idx++;
                aimbot_on_advance_hit_object();
                if (current_beatmap.hit_object_idx >= current_beatmap.hit_objects.size())
                    current_beatmap.ready = false;
            }
        }
    }
}
