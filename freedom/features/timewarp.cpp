#include "features/timewarp.h"

uintptr_t set_playback_rate_code_start = 0;
uintptr_t set_playback_rate_jump_back = 0;
uintptr_t set_playback_rate_original_mov_addr = 0;

uintptr_t check_timewarp_code_start = 0;
uintptr_t check_timewarp_value_1 = 64;
uintptr_t check_timewarp_value_2 = 32;

uintptr_t check_timewarp_hook_1 = 0;
uintptr_t check_timewarp_hook_1_jump_back = 0;

uintptr_t check_timewarp_hook_2 = 0;
uintptr_t check_timewarp_hook_2_jump_back = 0;

uintptr_t update_timing_code_start = 0;
uintptr_t update_timing_ptr_1 = 0;
uintptr_t update_timing_ptr_2 = 0;
uintptr_t update_timing_ptr_3 = 0;
uintptr_t update_timing_ptr_4 = 0;

Hook<Detour32> SetPlaybackRateHook;
Hook<Detour32> CheckTimewarpHook1;
Hook<Detour32> CheckTimewarpHook2;

void init_timewarp()
{
    if (set_playback_rate_code_start && check_timewarp_code_start)
    {
        SetPlaybackRateHook = Hook<Detour32>(set_playback_rate_code_start, (BYTE *)set_playback_rate, 10);
        CheckTimewarpHook1 = Hook<Detour32>(check_timewarp_hook_1, (BYTE *)set_check_timewarp_hook_1, 6);
        CheckTimewarpHook2 = Hook<Detour32>(check_timewarp_hook_2, (BYTE *)set_check_timewarp_hook_2, 6);
        if (cfg_timewarp_enabled)
            enable_timewarp_hooks();
    }
}

void enable_timewarp_hooks()
{
    SetPlaybackRateHook.Enable();
    CheckTimewarpHook1.Enable();
    CheckTimewarpHook2.Enable();
}

void disable_timewarp_hooks()
{
    SetPlaybackRateHook.Disable();
    CheckTimewarpHook1.Disable();
    CheckTimewarpHook2.Disable();
}

__declspec(naked) void set_playback_rate()
{
    __asm {
        push ebp
        push eax
        mov eax, dword ptr [cfg_timewarp_playback_rate]
        mov dword ptr [esp+0xC], eax
        mov eax, dword ptr [cfg_timewarp_playback_rate+0x4]
        mov dword ptr [esp+0x10], eax
        pop eax
        mov ebp,esp
        push esi
        push ebx
        mov ebx, dword ptr [set_playback_rate_original_mov_addr]
        mov esi, dword ptr [ebx]
        pop ebx
        jmp [set_playback_rate_jump_back]
    }
}

__declspec(naked) void set_check_timewarp_hook_1()
{
    __asm {
        mov eax, check_timewarp_value_1
        jmp [check_timewarp_hook_1_jump_back]
    }
}

__declspec(naked) void set_check_timewarp_hook_2()
{
    __asm {
        mov eax, check_timewarp_value_2
        jmp [check_timewarp_hook_2_jump_back]
    }
}
