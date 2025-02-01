#include "lazer/features/difficulty.h"

Parameter ar_parameter = {
    true,                   // lock
    10.0f,                  // value
    10.0f,                  // calculated_value
    OSU_BEATMAP_AR_OFFSET,  // offset
    "AR: %.1f",             // slider_fmt
    "AR Offsets Not Found", // error_message
    enable_ar_hooks,        // enable
    disable_ar_hooks,       // disable
    apply_mods_ar,          // apply_mods
    // bool found = false
};
