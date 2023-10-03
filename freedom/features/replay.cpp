#include "features/replay.h"

uintptr_t selected_replay_code_start = 0;
uintptr_t selected_replay_offset = 0;
uintptr_t selected_replay_hook_jump_back = 0;
uintptr_t selected_replay_ptr = 0;

Hook<Detour32> SelectedReplayHook;

void init_replay()
{
    if (selected_replay_offset)
    {
        SelectedReplayHook = Hook<Detour32>(selected_replay_offset, (BYTE *)notify_on_select_replay, 7);
        if (cfg_replay_enabled)
            SelectedReplayHook.Enable();
    }
}

void update_replay()
{
    if (cfg_replay_enabled && (cfg_replay_aim || cfg_replay_keys))
    {
        int32_t audio_time = *(int32_t *)audio_time_ptr;
        ReplayEntryData &entry = current_replay.current_entry();
        if (audio_time >= current_replay.replay_ms + entry.ms_since_last_frame)
        {
            static bool left = false;
            static bool right = false;
            if (current_replay.entries_idx < current_replay.entries.size())
            {
                if (cfg_replay_aim && entry.position.x > 0 && entry.position.y > 0)
                    move_mouse_to(entry.position.x, entry.position.y);
                if (cfg_replay_keys)
                {
                    switch (entry.keypresses)
                    {
                        case ReplayKeys::KEY_LEFT: {
                            if (!left) { send_keyboard_input(left_click[0], 0); left = true; }
                            if (right) { send_keyboard_input(right_click[0], KEYEVENTF_KEYUP); right = false; }
                        } break;
                        case ReplayKeys::KEY_RIGHT: {
                            if (left)   { send_keyboard_input(left_click[0], KEYEVENTF_KEYUP); left = false; }
                            if (!right) { send_keyboard_input(right_click[0], 0); right = true; }
                        } break;
                        case ReplayKeys::NO_KEY: {
                            if (left)  { send_keyboard_input(left_click[0], KEYEVENTF_KEYUP); left = false; }
                            if (right) { send_keyboard_input(right_click[0], KEYEVENTF_KEYUP); right = false; }
                        } break;
                        case ReplayKeys::KEY_LEFT_AND_RIGHT: {
                            if (!left)  { send_keyboard_input(left_click[0], 0); left = true; }
                            if (!right) { send_keyboard_input(right_click[0], 0); right = true; }
                        } break;
                        default:
                            break;
                    }
                }
                current_replay.replay_ms += entry.ms_since_last_frame;
                current_replay.entries_idx++;
            }
            else
            {
                if (cfg_replay_keys)
                {
                    if (left)  { send_keyboard_input(left_click[0], KEYEVENTF_KEYUP); left = false; }
                    if (right) { send_keyboard_input(right_click[0], KEYEVENTF_KEYUP); right = false; }
                }
                current_replay.replay_ms = 0;
                current_replay.entries_idx = 0;
            }
        }
    }
}

void replay_on_beatmap_load()
{
    if (current_replay.ready)
    {
        current_replay.replay_ms = 0;
        current_replay.entries_idx = 0;
    }
}

void enable_replay_hooks()
{
    enable_notify_hooks();
    SelectedReplayHook.Enable();
}

void disable_replay_hooks()
{
    disable_notify_hooks();
    SelectedReplayHook.Disable();
}

__declspec(naked) void notify_on_select_replay()
{
    __asm {
        mov eax, [esi+0x38]
        mov selected_replay_ptr, eax
        mov start_parse_replay, 1
        cmp dword ptr [eax+30], 0x00
        jmp [selected_replay_hook_jump_back]
    }
}
