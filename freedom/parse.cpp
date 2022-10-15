#include "parse.h"

ReplayEntryData& ReplayData::current_entry()
{
    return entries[entries_idx];
}

void ReplayData::clear()
{
    entries.clear();
    entries_idx = 0;
    replay_ms = 0;
    replay_bot_ms = 0;
}

Circle* BeatmapData::current_circle()
{
    return hit_objects[hit_object_idx];
}

void BeatmapData::clear()
{
    hit_object_idx = 0;
    ready = false;
    for (Circle *circle : hit_objects)
    {
        if (circle->type == HitObjectType::Slider)
        {
            Slider *slider = (Slider *)circle;
            slider->curves.clear();
            delete slider;
        }
        else
            delete circle;
    }
    hit_objects.clear();
}

bool parse_beatmap(uintptr_t osu_manager_ptr, BeatmapData &beatmap_data)
{
    beatmap_data.clear();

    if (osu_manager_ptr == 0)
        return false;

    uintptr_t osu_manager = *(uintptr_t *)(osu_manager_ptr);

    bool replay_mode = *(bool *)(osu_manager + 0x17A);
    if (replay_mode)
    {
        FR_INFO_FMT("skipping current beatmap: replay mode");
        return false;
    }

    uintptr_t hit_manager_ptr = *(uintptr_t *)(osu_manager + 0x40);
    uintptr_t hit_objects_list_ptr = *(uintptr_t *)(hit_manager_ptr + 0x48);
    uintptr_t hit_objects_list_items_ptr = *(uintptr_t *)(hit_objects_list_ptr + 0x4);
    int32_t hit_objects_count = *(int32_t *)(hit_manager_ptr + 0x90);

    beatmap_data.hit_objects.reserve(hit_objects_count);

    for (int32_t i = 0; i < hit_objects_count; ++i)
    {
        uintptr_t hit_object_ptr = *(uintptr_t *)(hit_objects_list_items_ptr + 0x8 + 0x4 * i);

        HitObjectType circle_type = *(HitObjectType *)(hit_object_ptr + 0x18);
        circle_type &= ~HitObjectType::ComboOffset;
        circle_type &= ~HitObjectType::NewCombo;

        Circle *circle;

        if (circle_type == HitObjectType::Slider)
        {
            Slider *slider = new Slider();
            uintptr_t curve_points_ptr = *(uintptr_t *)(hit_object_ptr + 0xC4);
            uintptr_t curve_points_list_ptr = *(uintptr_t *)(curve_points_ptr + 0x4);
            int32_t curve_points_count = *(int32_t *)(curve_points_ptr + 0xC);

            slider->curves.reserve(curve_points_count + 1);

            for (int32_t j = 0; j < curve_points_count; ++j)
            {
                uintptr_t curve_point = *(uintptr_t *)(curve_points_list_ptr + 0x8 + 0x4 * j);
                Vector2 p1(*(float *)(curve_point + 0x8), *(float *)(curve_point + 0xC));
                slider->curves.push_back(p1);
                if (j + 1 == curve_points_count)
                {
                    Vector2 p2(*(float *)(curve_point + 0x10), *(float *)(curve_point + 0x14));
                    slider->curves.push_back(p2);
                }
            }
            circle = (Circle *)slider;
        }
        else
        {
            circle = new Circle();
        }
        circle->start_time = *(int32_t *)(hit_object_ptr + 0x10);
        circle->end_time = *(int32_t *)(hit_object_ptr + 0x14);
        circle->type = circle_type;
        circle->position = Vector2(*(float *)(hit_object_ptr + 0x38), *(float *)(hit_object_ptr + 0x3C));
        beatmap_data.hit_objects.push_back(circle);
    }

    uintptr_t mods_ptr = *(uintptr_t *)(hit_manager_ptr + 0x34);
    int32_t encrypted_value = *(int32_t *)(mods_ptr + 0x08);
    int32_t decryption_key = *(int32_t *)(mods_ptr + 0x0C);
    beatmap_data.mods = (Mods)(encrypted_value ^ decryption_key);

    beatmap_data.ready = true;
    return true;
}
