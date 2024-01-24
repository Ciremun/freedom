#pragma once

#include "hook.h"
#include "config.h"

extern DWORD discord_rich_presence_code_start;
extern DWORD discord_rich_presence_jump_back;
extern DWORD discord_rich_presence_state_string_ptr;
extern DWORD discord_rich_presence_large_text_string_ptr;
extern DWORD discord_rich_presence_small_text_string_ptr;

void init_discord_rpc();

void enable_discord_rich_presence_hooks();
void disable_discord_rich_presence_hooks();

void set_discord_rich_presence();
