#pragma once

#include <mscoree.h>
#include <metahost.h>

#include "memory.h"

#include <vector>
#include <string>

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

bool prepare_methods();
bool init_clrhost();

bool variant_ok(VARIANT variant);
VARIANT invoke_csharp_method(const wchar_t *type_name, const wchar_t *method_name, const wchar_t *wchar_string_arg);
VARIANT invoke_csharp_method(const wchar_t *type_name, const wchar_t *method_name);
VARIANT invoke_csharp_method(const wchar_t *type_name, const wchar_t *method_name, SAFEARRAY* params);
