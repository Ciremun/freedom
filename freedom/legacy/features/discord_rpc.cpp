#include "legacy/features/discord_rpc.h"

intptr_t drpc_code_start = 0;
intptr_t drpc_jump_back = 0;
intptr_t drpc_state_string_ptr = NULL;
intptr_t drpc_state_string_gc_handle = NULL;
intptr_t drpc_large_text_string_ptr = NULL;
intptr_t drpc_large_text_string_gc_handle = NULL;
intptr_t drpc_small_text_string_ptr = NULL;
intptr_t drpc_small_text_string_gc_handle = NULL;

Hook<Detour32> DiscordRichPresenceHook;

void init_discord_rpc()
{
    drpc_code_start = get_set_presence_ptr();
    if (drpc_code_start)
    {
        drpc_jump_back = drpc_code_start + 0x5;
        DiscordRichPresenceHook = Hook<Detour32>(drpc_code_start, (BYTE *)set_drpc, 5);
        if (cfg_drpc_enabled)
        {
            enable_drpc_hooks();

            if (cfg_drpc_state[0] != '\0')
                set_discord_rpc_str(drpc_state_wchar, cfg_drpc_state, &drpc_state_string_ptr, &drpc_state_string_gc_handle);

            if (cfg_drpc_large_text[0] != '\0')
                set_discord_rpc_str(drpc_large_text_wchar, cfg_drpc_large_text, &drpc_large_text_string_ptr, &drpc_large_text_string_gc_handle);

            if (cfg_drpc_small_text[0] != '\0')
                set_discord_rpc_str(drpc_small_text_wchar, cfg_drpc_small_text, &drpc_small_text_string_ptr, &drpc_small_text_string_gc_handle);
        }
    }
}

void set_discord_rpc_str(wchar_t *w_str, char *c_str, intptr_t *output_str_ptr, intptr_t *output_str_gc_handle)
{
    if (*output_str_gc_handle)
        free_managed_string(*output_str_gc_handle);
    int wchars_count = MultiByteToWideChar(CP_UTF8, 0, c_str, -1, NULL, 0);
    int bytes_written = MultiByteToWideChar(CP_UTF8, 0, c_str, -1, w_str, wchars_count);
    w_str[bytes_written] = '\0';
    *output_str_ptr = allocate_managed_string(w_str, output_str_gc_handle);
}

void enable_drpc_hooks()
{
    DiscordRichPresenceHook.Enable();
}

void disable_drpc_hooks()
{
    DiscordRichPresenceHook.Disable();
}

__declspec(naked) void set_drpc()
{
    __asm {
        push esi
        mov eax, drpc_state_string_ptr
        mov dword ptr [edx+0x4], eax
        mov esi, drpc_large_text_string_ptr
        mov eax, [edx+0x10]
        mov dword ptr [eax+0x8], esi
        mov esi, drpc_small_text_string_ptr
        mov dword ptr [eax+0x10], esi
        pop esi
        push ebp
        mov ebp,esp
        push edi
        push esi
        jmp [drpc_jump_back]
    }
}
