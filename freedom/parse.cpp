#include "parse.h"

Circle BeatmapData::current_circle()
{
    return hit_objects[hit_object_idx];
}

void BeatmapData::clear()
{
    hit_object_idx = 0;
    ready = false;
    hit_objects.clear();
}

bool parse_beatmap(HANDLE hProc, uintptr_t osu_auth_base, BeatmapData &beatmap_data)
{
    beatmap_data.clear();

    static uintptr_t osu_player_ptr = internal_multi_level_pointer_dereference(hProc, osu_auth_base + osu_player_ptr_base_offset, osu_player_ptr_offsets);
    uintptr_t osu_manager_ptr = **(uintptr_t **)(osu_player_ptr + 0x8);
    uintptr_t hit_manager_ptr = *(uintptr_t *)(osu_manager_ptr + 0x40);
    uintptr_t hit_objects_list_ptr = *(uintptr_t *)(hit_manager_ptr + 0x48);
    uintptr_t hit_objects_list_items_ptr = *(uintptr_t *)(hit_objects_list_ptr + 0x4);
    int32_t hit_objects_count = *(int32_t *)(hit_manager_ptr + 0x90);

    for (int32_t i = 0; i < hit_objects_count; ++i)
    {
        uintptr_t hit_object_ptr = *(uintptr_t *)(hit_objects_list_items_ptr + 0x8 + 0x4 * i);
        Circle circle;
        circle.start_time = *(int32_t *)(hit_object_ptr + 0x10);
        circle.end_time = *(int32_t *)(hit_object_ptr + 0x14);
        beatmap_data.hit_objects.push_back(circle);
    }

    beatmap_data.ready = true;
    return true;
}
