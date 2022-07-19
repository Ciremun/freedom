#pragma once

#include "stdafx.h"

#include "iclr_debugging_library_provider.h"
#include "icor_debug_data_target.h"
#include "utility.h"
#include "code_start_target.h"
#include "freedom.h"

#include <vector>

#include <stdint.h>

bool code_start_for_class_methods(std::vector<CodeStartTarget> &targets);