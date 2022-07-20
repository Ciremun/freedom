#include "parse.h"

Circle BeatmapData::current_circle()
{
    return hit_objects[hit_object_idx];
}

void BeatmapData::clear()
{
    hit_object_idx = 0;
    slider_multiplier = 0.0f;
    parsed_successfully = false;
    timing_points.clear();
    hit_objects.clear();
}

bool parse_beatmap(const wchar_t *beatmap_path, BeatmapData &beatmap_data)
{
    beatmap_data.clear();
    File file = open_file(beatmap_path);
    if (file.handle == INVALID_HANDLE_VALUE)
        return false;
    if (!get_file_size(&file) || !map_file(&file))
    {
        close_file(file.handle);
        return false;
    }
    uint32_t line_start = 0;
    uint32_t line_end = 0;
    float slider_multiplier = 0.0f;
    BeatmapFileSection section = BeatmapFileSection::UNKNOWN;
    do
    {
        if (file.start[line_end] == '\n')
        {
            uint32_t line_len = line_end - line_start;
            if (!slider_multiplier && sscanf((const char *)(file.start + line_start), "SliderMultiplier:%f", &slider_multiplier))
                beatmap_data.slider_multiplier = slider_multiplier;
            else if (memcmp(file.start + line_start, "[HitObjects]", 12) == 0)
                section = BeatmapFileSection::HITOBJECTS;
            int32_t x, y, time, type, hit_sound;
            if (section != BeatmapFileSection::UNKNOWN && (file.start[line_start] == '\r' || file.start[line_start] == '\n'))
                section = BeatmapFileSection::UNKNOWN;
            if (section == BeatmapFileSection::HITOBJECTS)
            {
                if (sscanf((const char *)(file.start + line_start), "%d,%d,%d,%d,%d", &x, &y, &time, &type, &hit_sound))
                {
                    Circle circle;
                    circle.type = CircleType::POINT;
                    circle.hit_circle.time = time;
                    beatmap_data.hit_objects.push_back(circle);
                }
            }
            line_start = line_end + 1;
        }
        line_end++;
    } while (line_end < file.size);
    unmap_and_close_file(file);
    beatmap_data.parsed_successfully = true;
    return true;
}
