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

            int32_t repeats_count = *(int32_t *)(hit_object_ptr + 0x20);

            slider->curves.reserve(curve_points_count * repeats_count + 1);

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

            if (repeats_count > 1)
            {
                bool reversed = true;
                std::vector<Vector2<float>> reversed_curves;
                reversed_curves.reserve(slider->curves.size());
                reversed_curves.insert(reversed_curves.end(), slider->curves.rbegin(), slider->curves.rend());

                for (int k = 0; k < repeats_count - 1; ++k)
                {
                    if (reversed)
                        slider->curves.insert(slider->curves.end(), reversed_curves.begin(), reversed_curves.end());
                    else
                        slider->curves.insert(slider->curves.end(), reversed_curves.rbegin(), reversed_curves.rend());
                    reversed = !reversed;
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
    if (mods == Mods::None)
    {
        memcpy(buffer, "No Mod", 7);
        return;
    }
    size_t cursor = 0;
    const auto apply_mod = [](size_t &cursor, char *buffer, const char *mod){
        memcpy(buffer + cursor, mod, 2);
        cursor += 2;
    };
    if (mods & Mods::SpunOut)         apply_mod(cursor, buffer, "SO");
    if (mods & Mods::NoFail)          apply_mod(cursor, buffer, "NF");
    if (mods & Mods::Hidden)          apply_mod(cursor, buffer, "HD");
    if (mods & Mods::HardRock)        apply_mod(cursor, buffer, "HR");
    if (mods & Mods::HalfTime)        apply_mod(cursor, buffer, "HT");
    else if (mods & Mods::Nightcore)  apply_mod(cursor, buffer, "NC");
    else if (mods & Mods::DoubleTime) apply_mod(cursor, buffer, "DT");
    if (mods & Mods::Flashlight)      apply_mod(cursor, buffer, "FL");
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

    size_t compressed_data_size;
    uint8_t *compressed_data;

    if (compressed_data_ptr == 0)
    {
        // @@@ fixme refactor
        extern char osu_username[32];
        extern char osu_client_id[64];
        uint32_t replay_id = *(uint32_t *)(selected_replay_ptr + 0x4);
        if (replay_id && osu_username[0] != '\0' && osu_client_id[0] != '\0')
        {
            static const wchar_t* osu_domain = L"osu.ppy.sh";

            static std::vector<uint8_t> compressed_data_vec;
            compressed_data_vec.clear();
            compressed_data_vec.reserve(8192);

            static char replay_url[128];
            stbsp_snprintf(replay_url, 127, "/web/osu-getreplay.php?c=%u&m=0&u=%s&h=%s", replay_id, osu_username, osu_client_id);

            static wchar_t replay_url_w[256];
            int bytes_written = MultiByteToWideChar(CP_UTF8, 0, replay_url, 127, replay_url_w, 256);
            replay_url_w[bytes_written] = '\0';
            FR_INFO_FMT("replay_url_w: https://%S%S", osu_domain, replay_url_w);

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
                    FR_INFO_FMT("Error %u in WinHttpQueryDataAvailable.",
                            GetLastError());

                    pszOutBuffer = new char[dwSize];
                    ZeroMemory(pszOutBuffer, dwSize);

                    if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
                        FR_INFO_FMT("Error %u in WinHttpReadData.", GetLastError());

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
                printf("Error %d has occurred.\n", GetLastError());
                return false;
            }

            compressed_data_size = compressed_data_vec.size();
            compressed_data = &compressed_data_vec[0];
        }
        else
        {
            FR_INFO("compressed_data_ptr is null!");
            return false;
        }
    }
    else
    {
        compressed_data_size = *(uint32_t *)(compressed_data_ptr + 0x4);
        compressed_data = (uint8_t *)(compressed_data_ptr + 0x8);
    }

    FR_INFO_FMT("compressed_data_size: %zu", compressed_data_size);
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
        if (sscanf(replay_data_ptr, "%lld|%f|%f|%u", &entry.ms_since_last_frame, &entry.position.x, &entry.position.y, &entry.keypresses) == 4)
        {
            extern bool cfg_replay_hardrock;
            if (cfg_replay_hardrock)
                entry.position.y = std::abs(384.f - entry.position.y);
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
