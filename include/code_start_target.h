#pragma once

#include <stdint.h>

struct CodeStartTarget
{
    const wchar_t *class_;
    const wchar_t *method;
    uintptr_t start = 0;
};
