#include "features/discord_rpc.h"

DWORD discord_rich_presence_code_start = 0;
DWORD discord_rich_presence_jump_back = 0;
DWORD discord_rich_presence_state_string_ptr = NULL;
DWORD discord_rich_presence_large_text_string_ptr = NULL;
DWORD discord_rich_presence_small_text_string_ptr = NULL;

Hook<Detour32> DiscordRichPresenceHook;

void init_discord_rpc()
{
    VARIANT v = invoke_csharp_method(L"Freedom.Utils", L"GetSetPresencePtr");
    if (variant_ok(&v))
    {
        discord_rich_presence_code_start = v.intVal;
        discord_rich_presence_jump_back = discord_rich_presence_code_start + 0x5;
    }
    if (discord_rich_presence_code_start)
    {
        DiscordRichPresenceHook = Hook<Detour32>(discord_rich_presence_code_start, (BYTE *)set_discord_rich_presence, 5);
        if (cfg_discord_rich_presence_enabled)
        {
            enable_discord_rich_presence_hooks();

            if (cfg_discord_rich_presence_state[0] != '\0')
                set_discord_rpc_str(discord_rich_presence_state_wchar, cfg_discord_rich_presence_state, &discord_rich_presence_state_string_ptr);

            if (cfg_discord_rich_presence_large_text[0] != '\0')
                set_discord_rpc_str(discord_rich_presence_large_text_wchar, cfg_discord_rich_presence_large_text, &discord_rich_presence_large_text_string_ptr);

            if (cfg_discord_rich_presence_small_text[0] != '\0')
                set_discord_rpc_str(discord_rich_presence_small_text_wchar, cfg_discord_rich_presence_small_text, &discord_rich_presence_small_text_string_ptr);
        }
    }
}

void set_discord_rpc_str(wchar_t *w_str, char *c_str, DWORD *output_str_ptr)
{
    invoke_csharp_method(L"Freedom.Utils", L"FreeCSharpString", w_str);
    int wchars_count = MultiByteToWideChar(CP_UTF8, 0, c_str, -1, NULL, 0);
    int bytes_written = MultiByteToWideChar(CP_UTF8, 0, c_str, -1, w_str, wchars_count);
    w_str[bytes_written] = '\0';
    VARIANT v = invoke_csharp_method(L"Freedom.Utils", L"GetCSharpStringPtr", w_str);
    if (variant_ok(&v))
        *output_str_ptr = v.intVal;
}

void enable_discord_rich_presence_hooks()
{
    DiscordRichPresenceHook.Enable();
}

void disable_discord_rich_presence_hooks()
{
    DiscordRichPresenceHook.Disable();
}

__declspec(naked) void set_discord_rich_presence()
{
    __asm {
        push esi
        mov eax, discord_rich_presence_state_string_ptr
        mov dword ptr [edx+0x4], eax
        mov esi, discord_rich_presence_large_text_string_ptr
        mov eax, [edx+0x10]
        mov dword ptr [eax+0x8], esi
        mov esi, discord_rich_presence_small_text_string_ptr
        mov dword ptr [eax+0x10], esi
        pop esi
        push ebp
        mov ebp,esp
        push edi
        push esi
        jmp [discord_rich_presence_jump_back]
    }
}
