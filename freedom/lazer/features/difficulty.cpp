#include "lazer/features/difficulty.h"

uintptr_t on_beatmap_changed_ptr = 0;

typedef void(__fastcall* on_beatmap_changed_t)(void *, void *);
static on_beatmap_changed_t on_beatmap_changed;

DifficultySetting ar_setting = {
    true,                   // lock
    10.0f,                  // value
    "AR",                   // label
    "AR: %.1f",             // fmt
    enable_ar_hooks,        // enable
    disable_ar_hooks,       // disable
};

DifficultySetting cs_setting = {
    false,                  // lock
    4.0f,                   // value
    "CS",                   // label
    "CS: %.1f",             // fmt
    enable_cs_hooks,        // enable
    disable_cs_hooks,       // disable
};

DifficultySetting od_setting = {
    false,                  // lock
    8.0f,                   // value
    "OD",                   // label
    "OD: %.1f",             // fmt
    enable_od_hooks,        // enable
    disable_od_hooks,       // disable
};

static void __fastcall hk_on_beatmap_changed(void *_this, void *_value_changed)
{
    uintptr_t new_value = *(uintptr_t *)((uintptr_t)_value_changed + 0x10);
    uintptr_t beatmap_info = *(uintptr_t *)(new_value + 0x08);
    uintptr_t beatmap_difficulty = *(uintptr_t *)(beatmap_info + 0x28);
    if (ar_setting.enabled)
        *(float *)(beatmap_difficulty + 0x34) = ar_setting.value;
    else
        ar_setting.value = *(float *)(beatmap_difficulty + 0x34);
    on_beatmap_changed(_this, _value_changed);
}

static inline bool some_feature_requires_on_beatmap_changed_hook()
{
    return ar_setting.enabled || cs_setting.enabled || od_setting.enabled;
}

static inline void enable_on_beatmap_changed_hook()
{
    if (MH_EnableHook((LPVOID)on_beatmap_changed_ptr) != MH_OK)
        FR_ERROR("MH_EnableHook on_beatmap_changed_ptr");
}

static inline void disable_on_beatmap_changed_hook()
{
    if (!some_feature_requires_on_beatmap_changed_hook() && MH_DisableHook((LPVOID)on_beatmap_changed_ptr) != MH_OK)
        FR_ERROR("MH_DisableHook on_beatmap_changed_ptr");
}

void init_difficulty()
{
    // TODO(Ciremun): use print macros
    FR_INFO("on_beatmap_changed_ptr: %p", (void *)on_beatmap_changed_ptr);
    if (on_beatmap_changed_ptr)
    {
        if (MH_CreateHook(reinterpret_cast<void**>(on_beatmap_changed_ptr), &hk_on_beatmap_changed, reinterpret_cast<void**>(&on_beatmap_changed)) != MH_OK)
            FR_ERROR("MH_CreateHook on_beatmap_changed_ptr");
        else if (some_feature_requires_on_beatmap_changed_hook())
            enable_on_beatmap_changed_hook();
    }
}

void enable_ar_hooks() { enable_on_beatmap_changed_hook(); }
void disable_ar_hooks() { disable_on_beatmap_changed_hook(); }
void apply_mods_ar() {}

void enable_cs_hooks() { enable_on_beatmap_changed_hook(); }
void disable_cs_hooks() { disable_on_beatmap_changed_hook(); }
void apply_mods_cs() {}

void enable_od_hooks() { enable_on_beatmap_changed_hook(); }
void disable_od_hooks() { disable_on_beatmap_changed_hook(); }
void apply_mods_od() {}
