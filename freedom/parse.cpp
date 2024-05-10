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
    ready = false;
}

void ReplayData::toggle_hardrock()
{
    if (ready)
    {
        for (auto &entry : entries)
        {
            Vector2<float> playfield = screen_to_playfield(entry.position);
            playfield.y = std::abs(384.f - playfield.y);
            entry.position = playfield_to_screen(playfield);
        }
    }
}

Circle& BeatmapData::current_circle()
{
    return hit_objects[hit_object_idx];
}

void BeatmapData::clear()
{
    hit_object_idx = 0;
    ready = false;
    hit_objects.clear();
}

bool parse_beatmap(uintptr_t osu_manager_ptr, BeatmapData &beatmap_data)
{
    FR_INFO("[+] New Beatmap Detected");

    beatmap_data.clear();

    if (osu_manager_ptr == 0)
    {
        FR_ERROR("Parse Beatmap: osu_manager_ptr");
        return false;
    }

    uintptr_t osu_manager = *(uintptr_t *)(osu_manager_ptr);
    if (osu_manager == 0)
    {
        FR_ERROR("Parse Beatmap: osu_manager");
        return false;
    }

    bool replay_mode = *(bool *)(osu_manager + OSU_MANAGER_IS_REPLAY_MODE_OFFSET);
    if (replay_mode)
    {
        FR_INFO("Skipping current beatmap: replay mode");
        extern bool beatmap_loaded;
        beatmap_loaded = false;
        return false;
    }

    calc_playfield_from_window();

    uintptr_t hit_manager_ptr = *(uintptr_t *)(osu_manager + OSU_MANAGER_HIT_MANAGER_OFFSET);
    if (hit_manager_ptr == 0)
    {
        FR_ERROR("Parse Beatmap: hit_manager_ptr");
        return false;
    }

    uintptr_t hit_objects_list_ptr = *(uintptr_t *)(hit_manager_ptr + OSU_HIT_MANAGER_HIT_OBJECTS_LIST_OFFSET);
    if (hit_objects_list_ptr == 0)
    {
        FR_ERROR("Parse Beatmap: hit_objects_list_ptr");
        return false;
    }

    uintptr_t hit_objects_list_items_ptr = *(uintptr_t *)(hit_objects_list_ptr + 0x4);
    if (hit_objects_list_items_ptr == 0)
    {
        FR_ERROR("Parse Beatmap: hit_objects_list_items_ptr");
        return false;
    }

    int32_t hit_objects_count = *(int32_t *)(hit_manager_ptr + OSU_HIT_MANAGER_HIT_OBJECTS_COUNT_OFFSET);

    beatmap_data.hit_objects.reserve(hit_objects_count);

    for (int32_t i = 0; i < hit_objects_count; ++i)
    {
        uintptr_t hit_object_ptr = *(uintptr_t *)(hit_objects_list_items_ptr + 0x8 + 0x4 * i);
        if (hit_object_ptr == 0)
        {
            FR_ERROR("Parse Beatmap: hit_object_ptr");
            return false;
        }

        HitObjectType circle_type = *(HitObjectType *)(hit_object_ptr + OSU_HIT_OBJECT_CIRCLE_TYPE_OFFSET);
        circle_type &= ~HitObjectType::ComboOffset;
        circle_type &= ~HitObjectType::NewCombo;

        Circle circle;
        circle.start_time = *(int32_t *)(hit_object_ptr + OSU_HIT_OBJECT_START_TIME_OFFSET);
        circle.end_time = *(int32_t *)(hit_object_ptr + OSU_HIT_OBJECT_END_TIME_OFFSET);
        circle.type = circle_type;
        circle.position = Vector2(*(float *)(hit_object_ptr + OSU_HIT_OBJECT_POSITION_X_OFFSET), *(float *)(hit_object_ptr + OSU_HIT_OBJECT_POSITION_Y_OFFSET));
        beatmap_data.hit_objects.push_back(circle);
    }

    uintptr_t mods_ptr = *(uintptr_t *)(hit_manager_ptr + OSU_HIT_MANAGER_MODS_OFFSET);
    int32_t encrypted_value = *(int32_t *)(mods_ptr + OSU_HIT_MANAGER_MODS_ENC_VAL);
    int32_t decryption_key = *(int32_t *)(mods_ptr + OSU_HIT_MANAGER_MODS_DEC_KEY);
    beatmap_data.mods = (Mods)(encrypted_value ^ decryption_key);
    beatmap_data.hit_object_radius = *(float *)(hit_manager_ptr + OSU_HIT_MANAGER_HIT_OBJECT_RADIUS_OFFSET);

    float screen_ratio = window_size.y / 480.f;
    float game_height = 384.f * screen_ratio;
    float game_ratio = game_height / 384.f;
    beatmap_data.scaled_hit_object_radius = beatmap_data.hit_object_radius * game_ratio;

    FR_INFO("Hit Object Radius: %f", beatmap_data.hit_object_radius);
    FR_INFO("Scaled Hit Object Radius: %f", beatmap_data.scaled_hit_object_radius);

    // TODO(Ciremun): refactor
    uintptr_t selected_song_ptr = *(uintptr_t *)(osu_manager + OSU_MANAGER_BEATMAP_OFFSET);
    float od = *(float *)(selected_song_ptr + OSU_BEATMAP_OD_OFFSET);

    if (beatmap_data.mods & Mods::HardRock)  od = fmin(od * 1.4f, 10.f);
    else if (beatmap_data.mods & Mods::Easy) od /= 2.f;

    extern float od_window;

    od_window = 80.f - 6.f * od;
    od_window -= .5f;

    if (beatmap_data.mods & Mods::DoubleTime)    od_window *= 0.67f;
    else if (beatmap_data.mods & Mods::HalfTime) od_window *= 1.33f;

    // FIXME(Ciremun): refactor
    const auto rand_range_f = [](float f_min, float f_max) -> float
    {
        float scale = rand() / (float)RAND_MAX;
        return f_min + scale * (f_max - f_min);
    };

    extern float od_window_left_offset;
    extern float od_window_right_offset;
    srand(time(NULL));
    od_window_left_offset = -(od_window * rand_range_f(0.35f, 0.65f));
    od_window_right_offset = od_window * rand_range_f(0.15f, 0.85f);

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

char *mods_to_string(Mods &mods, char *buffer)
{
    if (mods == Mods::None)
    {
        memcpy(buffer, "No Mod", 7);
        return buffer;
    }
    size_t cursor = 0;
    const auto apply_mod = [](size_t &cursor, char *buffer, const char *mod){
        memcpy(buffer + cursor, mod, 2);
        cursor += 2;
    };
    if      (mods & Mods::SpunOut)     apply_mod(cursor, buffer, "SO");
    if      (mods & Mods::NoFail)      apply_mod(cursor, buffer, "NF");
    if      (mods & Mods::Hidden)      apply_mod(cursor, buffer, "HD");
    if      (mods & Mods::HalfTime)    apply_mod(cursor, buffer, "HT");
    else if (mods & Mods::Nightcore)   apply_mod(cursor, buffer, "NC");
    else if (mods & Mods::DoubleTime)  apply_mod(cursor, buffer, "DT");
    if      (mods & Mods::HardRock)    apply_mod(cursor, buffer, "HR");
    else if (mods & Mods::Easy)        apply_mod(cursor, buffer, "EZ");
    if      (mods & Mods::Flashlight)  apply_mod(cursor, buffer, "FL");
    if      (mods & Mods::TouchDevice) apply_mod(cursor, buffer, "TD");
    if      (mods & Mods::Perfect)     apply_mod(cursor, buffer, "PF");
    else if (mods & Mods::SuddenDeath) apply_mod(cursor, buffer, "SD");
    if      (mods & Mods::ScoreV2)     apply_mod(cursor, buffer, "V2");
    buffer[cursor] = '\0';
    return buffer;
}

static bool replay_beatmap_name(char *out)
{
    extern uintptr_t selected_song_ptr;
    if (selected_song_ptr)
    {
        uintptr_t song_str_ptr = 0;
        if (internal_memory_read(g_process, selected_song_ptr, &song_str_ptr))
        {
            song_str_ptr += 0x80;
            uintptr_t song_str = 0;
            if (internal_memory_read(g_process, song_str_ptr, &song_str))
            {
                song_str += 0x4;
                uint32_t song_str_length = 0;
                if (internal_memory_read(g_process, song_str, &song_str_length))
                {
                    song_str += 0x4;
                    int bytes_written = WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)song_str, song_str_length, out, 255, 0, 0);
                    out[bytes_written] = '\0';
                    return true;
                }
            }
        }
    }
    return false;
}

bool parse_replay(uintptr_t selected_replay_ptr, ReplayData &replay)
{
    FR_INFO("[+] New Replay Detected");
    replay.clear();

    if (!replay_beatmap_name(replay.song_name_u8))
        memcpy(replay.song_name_u8, "Unknown Beatmap", sizeof("Unknown Beatmap"));

    calc_playfield_from_window();

    uintptr_t author_str_obj = *(uintptr_t *)(selected_replay_ptr + OSU_REPLAY_AUTHOR_OFFSET);
    uint32_t author_str_length = *(uint32_t *)(author_str_obj + 0x4);
    wchar_t *author_str = (wchar_t *)(author_str_obj + 0x8);
    int bytes_written = WideCharToMultiByte(CP_UTF8, 0, author_str, author_str_length, replay.author, 31, 0, 0);
    replay.author[bytes_written] = '\0';

    uint16_t _300 = *(uint16_t *)(selected_replay_ptr + OSU_REPLAY_300_COUNT_OFFSET);
    uint16_t _100 = *(uint16_t *)(selected_replay_ptr + OSU_REPLAY_100_COUNT_OFFSET);
    uint16_t _50 = *(uint16_t *)(selected_replay_ptr + OSU_REPLAY_50_COUNT_OFFSET);
    uint16_t misses = *(uint16_t *)(selected_replay_ptr + OSU_REPLAY_MISS_COUNT_OFFSET);
    replay.accuracy = score_percent(_300, _100, _50, misses);

    replay.combo = *(uint32_t *)(selected_replay_ptr + OSU_REPLAY_COMBO_OFFSET);

    uintptr_t mods_ptr = *(uintptr_t *)(selected_replay_ptr + OSU_REPLAY_MODS_OFFSET);
    int32_t encrypted_value = *(int32_t *)(mods_ptr + 0x08);
    int32_t decryption_key = *(int32_t *)(mods_ptr + 0x0C);
    Mods mods = (Mods)(encrypted_value ^ decryption_key);
    mods_to_string(mods, replay.mods);

    uintptr_t compressed_data_ptr = *(uintptr_t *)(selected_replay_ptr + OSU_REPLAY_COMPRESSED_DATA_OFFSET);
    FR_PTR_INFO("selected_replay_ptr", selected_replay_ptr);

    size_t compressed_data_size;
    uint8_t *compressed_data;

    if (compressed_data_ptr == 0)
    {
        // @@@ fixme refactor
        extern char osu_username[32];
        extern char osu_client_id[64];
        int64_t replay_id = *(int64_t *)(selected_replay_ptr + 0x4);
        if (replay_id && osu_username[0] != '\0' && osu_client_id[0] != '\0')
        {
            static const wchar_t* osu_domain = L"osu.ppy.sh";

            static std::vector<uint8_t> compressed_data_vec;
            compressed_data_vec.clear();
            compressed_data_vec.reserve(8192);

            static char replay_url[128];
            stbsp_snprintf(replay_url, 127, "/web/osu-getreplay.php?c=%lld&m=0&u=%s&h=%s", replay_id, osu_username, osu_client_id);

            FR_INFO("Replay URL: %s", replay_url);

            static wchar_t replay_url_w[256];
            int bytes_written = MultiByteToWideChar(CP_UTF8, 0, replay_url, 127, replay_url_w, 256);
            replay_url_w[bytes_written] = '\0';

            DWORD dwSize = 0;
            DWORD dwDownloaded = 0;
            LPSTR pszOutBuffer;
            BOOL  bResults = FALSE;
            HINTERNET  hSession = NULL,
                        hConnect = NULL,
                        hRequest = NULL;

            hSession = WinHttpOpen(L"osu!",
                                    WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                    WINHTTP_NO_PROXY_NAME,
                                    WINHTTP_NO_PROXY_BYPASS, 0);

            if (hSession)
                hConnect = WinHttpConnect(hSession, osu_domain,
                                        INTERNET_DEFAULT_HTTPS_PORT, 0);

            if (hConnect)
                hRequest = WinHttpOpenRequest(hConnect, L"GET", replay_url_w,
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES,
                                            WINHTTP_FLAG_SECURE);

            if (hRequest)
                bResults = WinHttpSendRequest(hRequest,
                                            WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                            WINHTTP_NO_REQUEST_DATA, 0,
                                            0, 0);


            if (bResults)
                bResults = WinHttpReceiveResponse(hRequest, NULL);

            if (bResults)
            {
                do
                {
                    dwSize = 0;
                    if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
                    FR_INFO("Error %u in WinHttpQueryDataAvailable.",
                            GetLastError());

                    pszOutBuffer = new char[dwSize];
                    ZeroMemory(pszOutBuffer, dwSize);

                    if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
                        FR_INFO("Error %u in WinHttpReadData.", GetLastError());

                    for (DWORD i = 0; i < dwDownloaded; ++i)
                        compressed_data_vec.push_back(pszOutBuffer[i]);

                    delete[] pszOutBuffer;

                } while(dwSize > 0);
            }

            if (hRequest) WinHttpCloseHandle(hRequest);
            if (hConnect) WinHttpCloseHandle(hConnect);
            if (hSession) WinHttpCloseHandle(hSession);

            if (!bResults)
            {
                FR_INFO("Error %d has occurred.", GetLastError());
                return false;
            }

            compressed_data_size = compressed_data_vec.size();
            compressed_data = &compressed_data_vec[0];
        }
        else
        {
            FR_ERROR("Replay No Compressed Data Found");
            return false;
        }
    }
    else
    {
        compressed_data_size = *(uint32_t *)(compressed_data_ptr + 0x4);
        compressed_data = (uint8_t *)(compressed_data_ptr + 0x8);
    }

    FR_INFO("Replay Compressed Data Size: %zu", compressed_data_size);

    if (compressed_data_size == 0)
        return false;

    size_t replay_data_size = *(size_t *)&compressed_data[LZMA_HEADER_SIZE - 8];
    FR_INFO("Replay Data Size: %zu", replay_data_size);
    static std::vector<uint8_t> replay_data;
    replay_data.clear();
    replay_data.resize(replay_data_size);
    lzma_uncompress(&replay_data[0], &replay_data_size, compressed_data, &compressed_data_size);
    const char *replay_data_ptr = (const char *)&replay_data[0];
    size_t next_comma_position = 0;
    ReplayEntryData entry;
    replay.entries.reserve(replay_data_size / 4);
    extern bool cfg_replay_hardrock;
    bool hardrock = cfg_replay_hardrock;
    while (1)
    {
        if (sscanf(replay_data_ptr, "%lld|%f|%f|%u", &entry.ms_since_last_frame, &entry.position.x, &entry.position.y, &entry.keypresses) == 4)
        {
            if (!(entry.ms_since_last_frame == -12345 && entry.position.x == 0 && entry.position.y == 0))
            {
                if (hardrock)
                    entry.position.y = std::abs(384.f - entry.position.y);
                entry.position = playfield_to_screen(entry.position);
                replay.entries.push_back(entry);
            }
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
    FR_INFO("Replay Size: %zu", replay.entries.size());

    replay.ready = true;
    return true;
}
