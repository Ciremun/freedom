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
            enable_discord_rich_presence_hooks();
    }
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
