#pragma once

#include <mscoree.h>
#include <metahost.h>

#include "memory.h"

#include <vector>
#include <string>
#include <future>
#include <chrono>

#include <stdint.h>

#include "legacy/features/relax.h"
#include "legacy/features/aimbot.h"
#include "legacy/features/difficulty.h"
#include "legacy/features/discord_rpc.h"
#include "legacy/features/replay.h"
#include "legacy/features/score_multiplier.h"
#include "legacy/features/unmod_flashlight.h"
#include "legacy/features/timewarp.h"
#include "legacy/features/hidden_remover.h"

extern int prepared_methods_count;

bool init_clrhost();
bool prepare_methods();
intptr_t get_set_presence_ptr();
void free_managed_string(intptr_t gc_handle);
intptr_t allocate_managed_string(const wchar_t *str, intptr_t *gc_handle);
