#include "lazer/features/difficulty.h"

uintptr_t on_beatmap_changed_ptr = 0;

typedef void(__fastcall* on_beatmap_changed_t)(void *, void *);
static on_beatmap_changed_t on_beatmap_changed;

DifficultySetting ar_setting = {
    true,       // enabled
    10.0f,      // value
    "AR",       // label
    "AR: %.1f", // fmt
};

DifficultySetting cs_setting = {
    false,      // enabled
    4.0f,       // value
    "CS",       // label
    "CS: %.1f", // fmt
};

DifficultySetting od_setting = {
    false,      // enabled
    8.0f,       // value
    "OD",       // label
    "OD: %.1f", // fmt
};

static inline bool system_string_u8(uintptr_t string, char *out, int size)
{
    assert(string);
    assert(out);
    int32_t length = *(int32_t *)(string + 0x8);
    if (!length)
        return false;
    int bytes_written = WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)(string + 0xC), length, out, size, 0, 0);
    if (!bytes_written)
        return false;
    out[bytes_written] = '\0';
    return true;
}

static inline bool update_song_name(uintptr_t difficulty_name, uintptr_t metadata)
{
    assert(difficulty_name);
    assert(metadata);

    char difficulty[55] = {0};
    if (!system_string_u8(difficulty_name, difficulty, IM_ARRAYSIZE(difficulty)))
        return false;

    char title[100] = {0};
    uintptr_t title_ = *(uintptr_t *)(metadata + 0x18);
    uintptr_t title_unicode = *(uintptr_t *)(metadata + 0x20);
    if (!system_string_u8(title_, title, IM_ARRAYSIZE(title)) &&
        !system_string_u8(title_unicode, title, IM_ARRAYSIZE(title)))
        return false;

    char artist[100] = {0};
    uintptr_t artist_ = *(uintptr_t *)(metadata + 0x28);
    uintptr_t artist_unicode = *(uintptr_t *)(metadata + 0x30);
    if (!system_string_u8(artist_, artist, IM_ARRAYSIZE(artist)) &&
        !system_string_u8(artist_unicode, artist, IM_ARRAYSIZE(artist)))
        return false;

    ImFormatString(song_name_u8, IM_ARRAYSIZE(song_name_u8), "%s - %s [%s]", artist, title, difficulty);
    return true;
}

static void __fastcall hk_on_beatmap_changed(void *_this, void *_value_changed)
{
    assert(_value_changed);
    uintptr_t new_value = *(uintptr_t *)((uintptr_t)_value_changed + 0x10); assert(new_value);
    uintptr_t beatmap_info = *(uintptr_t *)(new_value + 0x08); assert(beatmap_info);
    uintptr_t beatmap_difficulty = *(uintptr_t *)(beatmap_info + 0x28); assert(beatmap_difficulty);
    ar_setting.enabled ? *(float *)(beatmap_difficulty + 0x34) = ar_setting.value : ar_setting.value = *(float *)(beatmap_difficulty + 0x34);
    cs_setting.enabled ? *(float *)(beatmap_difficulty + 0x2C) = cs_setting.value : cs_setting.value = *(float *)(beatmap_difficulty + 0x2C);
    od_setting.enabled ? *(float *)(beatmap_difficulty + 0x30) = od_setting.value : od_setting.value = *(float *)(beatmap_difficulty + 0x30);
    uintptr_t difficulty_name = *(uintptr_t *)(beatmap_info + 0x18);
    uintptr_t metadata = *(uintptr_t *)(beatmap_info + 0x30);
    if (!update_song_name(difficulty_name, metadata))
        FR_ERROR("update_song_name failed");
    on_beatmap_changed(_this, _value_changed);
}

void init_difficulty()
{
    FR_INFO("on_beatmap_changed_ptr: %" PRIXPTR, on_beatmap_changed_ptr);
    if (on_beatmap_changed_ptr)
    {
        if (MH_CreateHook(reinterpret_cast<void**>(on_beatmap_changed_ptr), &hk_on_beatmap_changed, reinterpret_cast<void**>(&on_beatmap_changed)) != MH_OK)
            FR_ERROR("CreateHook on_beatmap_changed_ptr");
        else
            enable_difficulty_hook();
    }
}

void enable_difficulty_hook()
{
    if (MH_EnableHook((LPVOID)on_beatmap_changed_ptr) != MH_OK)
        FR_ERROR("EnableHook on_beatmap_changed_ptr");
}

void disable_difficulty_hook()
{
    if (MH_DisableHook((LPVOID)on_beatmap_changed_ptr) != MH_OK)
        FR_ERROR("DisableHook on_beatmap_changed_ptr");
}
