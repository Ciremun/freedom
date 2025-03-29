#include "lazer/events/beatmap_changed.h"

typedef void(__fastcall* on_beatmap_changed_t)(void *, void *);
static on_beatmap_changed_t o_on_beatmap_changed;
uintptr_t on_beatmap_changed_ptr = 0;

static inline bool system_string_u8(uintptr_t string, char *out, int size)
{
    assert(string);
    assert(out);
    int32_t length = *(int32_t *)(string + SYSTEM_STRING_LENGTH_OFFSET);
    if (!length)
        return false;
    wchar_t *str = (wchar_t *)(string + SYSTEM_STRING_FIRST_CHAR_OFFSET);
    int bytes_written = WideCharToMultiByte(CP_UTF8, 0, str, length, out, size, 0, 0);
    if (!bytes_written)
        return false;
    out[bytes_written] = '\0';
    return true;
}

// TODO(Ciremun): draw unicode stuff
static inline bool update_song_name(uintptr_t difficulty_name, uintptr_t metadata)
{
    assert(difficulty_name);
    assert(metadata);

    char difficulty[55] = {0};
    if (!system_string_u8(difficulty_name, difficulty, IM_ARRAYSIZE(difficulty)))
        return false;

    char title[100] = {0};
    uintptr_t title_ = *(uintptr_t *)(metadata + OSU_METADATA_TITLE_OFFSET);
    // uintptr_t title_unicode = *(uintptr_t *)(metadata + OSU_METADATA_TITLE_UNICODE_OFFSET);
    if (!system_string_u8(title_, title, IM_ARRAYSIZE(title)))
        return false;

    char artist[100] = {0};
    uintptr_t artist_ = *(uintptr_t *)(metadata + OSU_METADATA_ARTIST_OFFSET);
    // uintptr_t artist_unicode = *(uintptr_t *)(metadata + OSU_METADATA_ARTIST_UNICODE_OFFSET);
    if (!system_string_u8(artist_, artist, IM_ARRAYSIZE(artist)))
        return false;

    ImFormatString(song_name_u8, IM_ARRAYSIZE(song_name_u8), "%s - %s [%s]", artist, title, difficulty);
    return true;
}

// osu.Game.OsuGameBase, ValueChangedEvent<WorkingBeatmap>
static __declspec(noinline) void __fastcall hk_on_beatmap_changed(void *_this, void *_value_changed)
{
    assert(_value_changed);
    uintptr_t new_value = *(uintptr_t *)((uintptr_t)_value_changed + OSU_VALUE_CHANGED_NEW_VALUE_OFFSET); assert(new_value);
    uintptr_t beatmap_info = *(uintptr_t *)(new_value + OSU_NEW_VALUE_BEATMAP_INFO_OFFSET); assert(beatmap_info);
    uintptr_t beatmap_difficulty = *(uintptr_t *)(beatmap_info + OSU_BEATMAP_INFO_BEATMAP_DIFFICULTY_OFFSET); assert(beatmap_difficulty);
    uintptr_t difficulty_name = *(uintptr_t *)(beatmap_info + OSU_BEATMAP_INFO_DIFFICULTY_NAME_OFFSET);
    uintptr_t metadata = *(uintptr_t *)(beatmap_info + OSU_BEATMAP_INFO_METADATA_OFFSET);
    if (!update_song_name(difficulty_name, metadata))
        FR_ERROR("update_song_name failed");
    if (!ar_setting.enabled) ar_setting.value = *(float *)(beatmap_difficulty + OSU_BEATMAP_DIFFICULTY_AR_OFFSET);
    if (!cs_setting.enabled) cs_setting.value = *(float *)(beatmap_difficulty + OSU_BEATMAP_DIFFICULTY_CS_OFFSET);
    if (!od_setting.enabled) od_setting.value = *(float *)(beatmap_difficulty + OSU_BEATMAP_DIFFICULTY_OD_OFFSET);
    if (!dr_setting.enabled) dr_setting.value = *(float *)(beatmap_difficulty + OSU_BEATMAP_DIFFICULTY_DR_OFFSET);
    o_on_beatmap_changed(_this, _value_changed);
}

void init_on_beatmap_changed()
{
    FR_INFO("on_beatmap_changed_ptr: %" PRIXPTR, on_beatmap_changed_ptr);
    if (on_beatmap_changed_ptr)
    {
        if (MH_CreateHook(reinterpret_cast<void**>(on_beatmap_changed_ptr), &hk_on_beatmap_changed, reinterpret_cast<void**>(&o_on_beatmap_changed)) != MH_OK)
            FR_ERROR("CreateHook on_beatmap_changed_ptr");
        else
            enable_beatmap_changed_hook();
    }
}

void enable_beatmap_changed_hook()
{
    if (MH_EnableHook((LPVOID)on_beatmap_changed_ptr) != MH_OK)
        FR_ERROR("EnableHook on_beatmap_changed_ptr");
}
