#pragma once

#include "legacy/hook.h"
#include "legacy/clrhost.h"

#include "ui/config.h"

extern intptr_t drpc_code_start;
extern intptr_t drpc_jump_back;
extern intptr_t drpc_state_string_ptr;
extern intptr_t drpc_state_string_gc_handle;
extern intptr_t drpc_large_text_string_ptr;
extern intptr_t drpc_large_text_string_gc_handle;
extern intptr_t drpc_small_text_string_ptr;
extern intptr_t drpc_small_text_string_gc_handle;

extern Hook<Detour32> DiscordRichPresenceHook;

void init_discord_rpc();

void set_discord_rpc_str(wchar_t *w_str, char *c_str, intptr_t *output_str_ptr, intptr_t *output_str_gc_handle);
void enable_drpc_hooks();
void disable_drpc_hooks();
void set_drpc();
