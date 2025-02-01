#pragma once

#include <stdint.h>

struct Parameter
{
    bool lock;
    float value;
    float calculated_value;
    uintptr_t offset;
    const char *slider_fmt;
    const char *error_message;
    void (*enable)();
    void (*disable)();
    void (*apply_mods)();
    bool found = false;
};
