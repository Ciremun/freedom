#pragma once

#include "stdafx.h"

#include "dotnet/iclr_debugging_library_provider.h"
#include "dotnet/icor_debug_data_target.h"
#include "mem.h"
#include "freedom.h"

#include <vector>

#include <stdint.h>

struct CodeStartTarget
{
    const wchar_t *class_;
    const wchar_t *method;
    uintptr_t start = 0;
};

bool code_start_for_class_methods(std::vector<CodeStartTarget> &targets);
