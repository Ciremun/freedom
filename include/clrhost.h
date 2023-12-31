#pragma once

#include "stdafx.h"

#include "mem.h"

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

extern bool prepared_all_methods;
extern bool prepared_methods;

bool prepare_all_methods_slow();
bool prepare_all_methods_fast();
void get_classmethods_from_addrs();
bool load_csharp_assembly();

bool variant_ok(void *variant_ptr);
VARIANT invoke_csharp_method(const wchar_t *type_name, const wchar_t *method_name, const wchar_t *wchar_string_arg);
VARIANT invoke_csharp_method(const wchar_t *type_name, const wchar_t *method_name);
VARIANT invoke_csharp_method(const wchar_t *type_name, const wchar_t *method_name, SAFEARRAY* params);
