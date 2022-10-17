#define LZMA_IMPLEMENTATION
#include "lzma.h"

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

static float score_percent(uint16_t _300, uint16_t _100, uint16_t _50, uint16_t misses)
{
    float percent = .0f;
    int objects_count = _300 + _100 + _50 + misses;
    if (objects_count > 0)
        percent = (_300 * 300 + _100 * 100 + _50 * 50) / (objects_count * 300.f) * 100.f;
    return percent;
}

static void mods_to_string(Mods &mods, char *buffer)
{
    if (mods & Mods::None)
    {
        memcpy(buffer, "nomod", 6);
        return;
    }
    size_t cursor = 0;
    const auto apply_mod = [](size_t &cursor, char *buffer, const char *mod){
        memcpy(buffer + cursor, mod, 2);
        cursor += 2;
    };
    if (mods & Mods::SpunOut)
        apply_mod(cursor, buffer, "SO");
    if (mods & Mods::NoFail)
        apply_mod(cursor, buffer, "NF");
    if (mods & Mods::Hidden)
        apply_mod(cursor, buffer, "HD");
    if (mods & Mods::HardRock)
        apply_mod(cursor, buffer, "HR");
    if (mods & Mods::HalfTime)
        apply_mod(cursor, buffer, "HT");
    else if (mods & Mods::Nightcore)
        apply_mod(cursor, buffer, "NC");
    else if (mods & Mods::DoubleTime)
        apply_mod(cursor, buffer, "DT");
    if (mods & Mods::Flashlight)
        apply_mod(cursor, buffer, "FL");
    buffer[cursor] = '\0';
}

bool parse_replay(uintptr_t selected_replay_ptr, ReplayData &replay)
{
    replay.clear();

    extern char song_name_u8[256];
    memcpy(replay.song_name_u8, song_name_u8, 256);

    uintptr_t author_str_obj = *(uintptr_t *)(selected_replay_ptr + 0x28);
    uint32_t author_str_length = *(uint32_t *)(author_str_obj + 0x4);
    wchar_t *author_str = (wchar_t *)(author_str_obj + 0x8);
    int bytes_written = WideCharToMultiByte(CP_UTF8, 0, author_str, author_str_length, replay.author, 31, 0, 0);
    replay.author[bytes_written] = '\0';

    uint16_t _300 = *(uint16_t *)(selected_replay_ptr + 0x8A);
    uint16_t _100 = *(uint16_t *)(selected_replay_ptr + 0x88);
    uint16_t _50 = *(uint16_t *)(selected_replay_ptr + 0x8C);
    uint16_t misses = *(uint16_t *)(selected_replay_ptr + 0x92);
    replay.accuracy = score_percent(_300, _100, _50, misses);

    replay.combo = *(uint32_t *)(selected_replay_ptr + 0x68);

    uintptr_t mods_ptr = *(uintptr_t *)(selected_replay_ptr + 0x1C);
    int32_t encrypted_value = *(int32_t *)(mods_ptr + 0x08);
    int32_t decryption_key = *(int32_t *)(mods_ptr + 0x0C);
    Mods mods = (Mods)(encrypted_value ^ decryption_key);
    mods_to_string(mods, replay.mods);

    uintptr_t compressed_data_ptr = *(uintptr_t *)(selected_replay_ptr + 0x30);
    FR_PTR_INFO("selected_replay_ptr", selected_replay_ptr);

    if (compressed_data_ptr == 0)
    {
        FR_INFO("compressed_data_ptr is null!");
        return false;
    }

    size_t compressed_data_size = *(uint32_t *)(compressed_data_ptr + 0x4);
    FR_INFO_FMT("compressed_data_size: %zu", compressed_data_size);
    uint8_t *compressed_data = (uint8_t *)(compressed_data_ptr + 0x8);
    size_t replay_data_size = *(size_t *)&compressed_data[LZMA_HEADER_SIZE - 8];
    FR_INFO_FMT("replay_data_size: %zu", replay_data_size);
    static std::vector<uint8_t> replay_data;
    replay_data.reserve(replay_data_size);
    lzma_uncompress(&replay_data[0], &replay_data_size, compressed_data, &compressed_data_size);
    const char *replay_data_ptr = (const char *)&replay_data[0];
    size_t next_comma_position = 0;
    ReplayEntryData entry;
    while (entry.ms_since_last_frame != -12345)
    {
        if (sscanf(replay_data_ptr, "%lld|%f|%f|%d", &entry.ms_since_last_frame, &entry.position.x, &entry.position.y, &entry.keypresses) == 4)
        {
            entry.position = playfield_to_screen(entry.position);
            replay.entries.push_back(entry); // fixme - reserve
        }
        else
            break;
        while (next_comma_position < replay_data_size)
            if (replay_data[++next_comma_position] == ',')
                break;
        if (next_comma_position >= replay_data_size)
            break;
        replay_data_ptr += (const char *)&replay_data[next_comma_position] - replay_data_ptr + 1;
    }
    FR_INFO_FMT("replay.size: %zu", replay.entries.size());

    replay.ready = true;
    return true;
}
