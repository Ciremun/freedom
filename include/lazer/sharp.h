#pragma once

#include <windows.h>

#include <stdint.h>

uint32_t *token_to_rva(uintptr_t base, uint32_t token);
