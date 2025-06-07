#pragma once

#include <windows.h>

#include <stdint.h>
#include <inttypes.h>

#include "ui/debug_log.h"

uint32_t *token_to_rva(uintptr_t base, uint32_t token);
