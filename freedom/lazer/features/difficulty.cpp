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

static void __fastcall hk_on_beatmap_changed(void *_this, void *_value_changed)
{
    uintptr_t new_value = *(uintptr_t *)((uintptr_t)_value_changed + 0x10);
    uintptr_t beatmap_info = *(uintptr_t *)(new_value + 0x08);
    uintptr_t beatmap_difficulty = *(uintptr_t *)(beatmap_info + 0x28);
    ar_setting.enabled ? *(float *)(beatmap_difficulty + 0x34) = ar_setting.value : ar_setting.value = *(float *)(beatmap_difficulty + 0x34);
    cs_setting.enabled ? *(float *)(beatmap_difficulty + 0x2C) = cs_setting.value : cs_setting.value = *(float *)(beatmap_difficulty + 0x2C);
    od_setting.enabled ? *(float *)(beatmap_difficulty + 0x30) = od_setting.value : od_setting.value = *(float *)(beatmap_difficulty + 0x30);
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
