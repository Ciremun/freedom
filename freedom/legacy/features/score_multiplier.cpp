#include "legacy/features/score_multiplier.h"

uintptr_t score_multiplier_code_start = 0;
uintptr_t score_multiplier_offset = 0;
uintptr_t score_multiplier_hook_jump_back = 0;

Hook<Detour32> ScoreMultiplierHook;

void init_score_multiplier()
{
    if (score_multiplier_offset)
    {
        ScoreMultiplierHook = Hook<Detour32>(score_multiplier_offset, (BYTE *)set_score_multiplier, 5);
        if (cfg_score_multiplier_enabled)
            enable_score_multiplier_hooks();
    }
}

void enable_score_multiplier_hooks()
{
    ScoreMultiplierHook.Enable();
}

void disable_score_multiplier_hooks()
{
    ScoreMultiplierHook.Disable();
}

__declspec(naked) void set_score_multiplier()
{
    __asm {
        fld dword ptr [cfg_score_multiplier_value]
        cmp edx, 0x04
        jmp [score_multiplier_hook_jump_back]
    }
}
