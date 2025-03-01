#include "legacy/features/unmod_flashlight.h"

uintptr_t update_flashlight_code_start = 0;
uint8_t update_flashlight_original_byte = update_flashlight_func_sig[0].m_value;

uintptr_t check_flashlight_code_start = 0;
uint8_t check_flashlight_original_byte = check_flashlight_func_sig[0].m_value;

void init_unmod_flashlight()
{
    if (update_flashlight_code_start)
    {
        if (cfg_flashlight_enabled)
            enable_flashlight_hooks();
    }
}

void set_flashlight_alpha_value(float value)
{
    if (osu_manager_ptr)
    {
        uintptr_t osu_manager = *(uintptr_t *)(osu_manager_ptr);
        if (osu_manager)
        {
            uintptr_t osu_ruleset_ptr = *(uintptr_t *)(osu_manager + OSU_MANAGER_RULESET_PTR_OFFSET);
            if (osu_ruleset_ptr)
            {
                uintptr_t flashlight_sprite_manager = *(uintptr_t *)(osu_ruleset_ptr + OSU_RULESET_FLASHLIGHT_SPRITE_MANAGER_OFFSET);
                if (flashlight_sprite_manager)
                    *(float *)(flashlight_sprite_manager + OSU_FLASHLIGHT_SPRITE_MANAGER_ALPHA_OFFSET) = value;
            }
        }
    }
}

void unmod_flashlight_on_beatmap_load()
{
    if (cfg_flashlight_enabled && osu_manager_ptr)
        set_flashlight_alpha_value(0.f);
}

void enable_flashlight_hooks()
{
    enable_notify_hooks();
    if (update_flashlight_code_start)
    {
        update_flashlight_original_byte = *(uint8_t *)update_flashlight_code_start;
        *(uint8_t *)update_flashlight_code_start = (uint8_t)0xC3; // ret
    }
    if (check_flashlight_code_start)
    {
        check_flashlight_original_byte = *(uint8_t *)check_flashlight_code_start;
        *(uint8_t *)check_flashlight_code_start = (uint8_t)0xC3; // ret
    }
    set_flashlight_alpha_value(0.f);
}

void disable_flashlight_hooks()
{
    disable_notify_hooks();
    if (update_flashlight_code_start)
        *(uint8_t *)update_flashlight_code_start = update_flashlight_original_byte;
    if (check_flashlight_code_start)
        *(uint8_t *)check_flashlight_code_start = check_flashlight_original_byte;
    set_flashlight_alpha_value(1.f);
}
