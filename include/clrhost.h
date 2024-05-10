#pragma once

#include <mscoree.h>
#include <metahost.h>

#include "memory.h"

#include <vector>
#include <string>
#include <future>
#include <chrono>

#include <stdint.h>

#include "features/relax.h"
#include "features/aimbot.h"
#include "features/difficulty.h"
#include "features/discord_rpc.h"
#include "features/replay.h"
#include "features/score_multiplier.h"
#include "features/unmod_flashlight.h"
#include "features/timewarp.h"
#include "features/hidden_remover.h"

extern int prepared_methods_count;

bool init_clrhost();
bool prepare_methods();
intptr_t get_set_presence_ptr();
void free_managed_string(intptr_t gc_handle);
intptr_t allocate_managed_string(const wchar_t *str, intptr_t *gc_handle);
