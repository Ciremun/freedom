#include "parse.h"

Circle BeatmapData::current_circle()
{
    return hit_objects[hit_object_idx];
}

TimingPoint BeatmapData::current_timing_point()
{
    return timing_points[timing_point_idx];
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
            else if (memcmp(file.start + line_start, "[TimingPoints]", 14) == 0)
                section = BeatmapFileSection::TIMING_POINTS;

            if (section != BeatmapFileSection::UNKNOWN && (file.start[line_start] == '\r' || file.start[line_start] == '\n'))
                section = BeatmapFileSection::UNKNOWN;

            if (section == BeatmapFileSection::HITOBJECTS)
            {
                int32_t x, y, time, type, hit_sound, end_time;
                char curve_type;
                if (sscanf((const char *)(file.start + line_start), "%d,%d,%d,%d,%d,%d", &x, &y, &time, &type, &hit_sound, &end_time) == 6 && end_time > 12)
                {
                    FR_INFO_FMT("found spinner! time: %d, end_time: %d", time, end_time);
                    Circle circle;
                    circle.type = CircleType::SPINNER;
                    circle.spinner.time = time;
                    circle.spinner.end_time = end_time;
                    beatmap_data.hit_objects.push_back(circle);
                }
                else if (sscanf((const char *)(file.start + line_start), "%d,%d,%d,%d,%d,%c", &x, &y, &time, &type, &hit_sound, &curve_type) == 6 && ('A' <= curve_type && curve_type <= 'Z'))
                {
                    uint32_t curve_points_end_idx = line_start;
                    while (file.start[curve_points_end_idx++] != '|' && curve_points_end_idx < line_end)
                        continue;
                    while (file.start[curve_points_end_idx++] != ',' && curve_points_end_idx < line_end)
                        continue;
                    int32_t slides;
                    double length;
                    if (sscanf((const char *)(file.start + curve_points_end_idx), "%d,%lf", &slides, &length) == 2)
                    {
                        // FR_INFO_FMT("found slider: %d %d %lf", time, slides, length);
                        Circle circle;
                        circle.type = CircleType::SLIDER;
                        circle.slider.time = time;
                        circle.slider.slides = slides;
                        circle.slider.length = length;
                        beatmap_data.hit_objects.push_back(circle);
                    }
                }
                else if (sscanf((const char *)(file.start + line_start), "%d,%d,%d,%d,%d", &x, &y, &time, &type, &hit_sound) == 5)
                {
                    // FR_INFO_FMT("found point! %d", time);
                    Circle circle;
                    circle.type = CircleType::POINT;
                    circle.hit_circle.time = time;
                    beatmap_data.hit_objects.push_back(circle);
                }
            }
            else if (section == BeatmapFileSection::TIMING_POINTS)
            {
                int32_t time, meter, sample_set, sample_index, volume, uninherited, effects;
                double beat_length;
                if (sscanf((const char *)(file.start + line_start), "%d,%lf,%d,%d,%d,%d,%d,%d", &time, &beat_length, &meter, &sample_set, &sample_index, &volume, &uninherited, &effects) == 8)
                {
                    TimingPoint timing_point;
                    timing_point.time = time;
                    timing_point.beat_length = beat_length;
                    timing_point.uninherited = (bool)uninherited;
                    beatmap_data.timing_points.push_back(timing_point);
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
