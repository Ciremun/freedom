#include "lazer/scan.h"

#define PATTERN_SCAN(out, signature, start) if (!out) out = pattern::find<signature>({ start, signature.size() })

uintptr_t on_beatmap_changed_ptr = 0;

struct CodeStart
{
    const char *name;
    uintptr_t *ptr;
};

CodeStart all_code_starts[] = {
    { .name = "OnBeatmapChanged", .ptr = &on_beatmap_changed_ptr },
};

inline bool all_code_starts_found()
{
    for (const auto &code_start : all_code_starts)
        if (!*code_start.ptr)
            return false;
    return true;
}

static void scan_for_code_starts()
{
    int alignment = 16;
    _MEMORY_BASIC_INFORMATION mbi;
    for (uint8_t *p = (uint8_t *)0x700000000000; VirtualQuery(p, &mbi, sizeof(mbi)); p += mbi.RegionSize)
    {
        if (mbi.State != MEM_COMMIT)
            continue;

        if (mbi.Protect != PAGE_EXECUTE_READ)
            continue;

        for (unsigned int idx = 0; idx != mbi.RegionSize / alignment; ++idx)
        {
            uint8_t *opcodes = (uint8_t *)((uintptr_t)mbi.BaseAddress + idx * alignment);
            PATTERN_SCAN(on_beatmap_changed_ptr, on_beatmap_changed_sig, opcodes);
            if (all_code_starts_found())
                return;
        }
    }
}

#include "minhook.h"
#include <inttypes.h>

typedef void(__fastcall* on_beatmap_changed_t)(void *, void *);
on_beatmap_changed_t on_beatmap_changed;

void __fastcall hk_on_beatmap_changed(void *_this, void *_value_changed)
{
    uintptr_t new_value = *(uintptr_t *)((uintptr_t)_value_changed + 0x10);
    uintptr_t beatmap_info = *(uintptr_t *)(new_value + 0x08);
    uintptr_t beatmap_difficulty = *(uintptr_t *)(beatmap_info + 0x28);
    *(float *)(beatmap_difficulty + 0x34) = 10.0f;
    on_beatmap_changed(_this, _value_changed);
}

void init_hooks()
{
    uintptr_t osu_game_dll_base = GetModuleBaseAddress(L"osu.Game.dll");
    if (osu_game_dll_base != NULL)
    {
        // TODO(Ciremun): offsets header
        BYTE ff = (BYTE)0xFF;
        internal_memory_patch((BYTE *)(osu_game_dll_base + 0xBD38), &ff, sizeof(BYTE));
    }
    else
        FR_ERROR("GetModuleBaseAddress osu.Game.dll");

    scan_for_code_starts();
    // TODO(Ciremun): use print macros
    FR_INFO("on_beatmap_changed_ptr: %p", (void *)on_beatmap_changed_ptr);

    if (on_beatmap_changed_ptr)
    {
        if (MH_CreateHook(reinterpret_cast<void**>(on_beatmap_changed_ptr), &hk_on_beatmap_changed, reinterpret_cast<void**>(&on_beatmap_changed)) != MH_OK) {
            FR_ERROR("MH_CreateHook on_beatmap_changed_ptr");
            goto ggs;
        }

        if (MH_EnableHook((LPVOID)on_beatmap_changed_ptr) != MH_OK) {
            FR_ERROR("MH_EnableHook on_beatmap_changed_ptr");
            goto ggs;
        }
    }

ggs:
    return;
}
