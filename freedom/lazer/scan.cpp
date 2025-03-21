#include "lazer/scan.h"

#define PATTERN_SCAN(out, signature, start) if (!out) out = pattern::find<signature>({ start, signature.size() })

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
    SIZE_T alignment = 16;
    _MEMORY_BASIC_INFORMATION mbi;
    for (uint8_t *p = (uint8_t *)0x700000000000; VirtualQuery(p, &mbi, sizeof(mbi)); p += mbi.RegionSize)
    {
        if (mbi.State != MEM_COMMIT)
            continue;

        if (mbi.Protect != PAGE_EXECUTE_READ)
            continue;

        for (SIZE_T idx = 0; idx != mbi.RegionSize / alignment; ++idx)
        {
            uint8_t *opcodes = (uint8_t *)((uintptr_t)mbi.BaseAddress + idx * alignment);
            PATTERN_SCAN(on_beatmap_changed_ptr, on_beatmap_changed_sig, opcodes);
            if (all_code_starts_found())
                return;
        }
    }
}

static inline void patch_osu_game_dll(uintptr_t base)
{
    if (!base)
        return;
    // TODO(Ciremun): offsets header
    BYTE ff = (BYTE)0xFF;
    internal_memory_patch((BYTE *)(base + 0xBE84), &ff, sizeof(BYTE));
}

void init_hooks()
{
    uintptr_t osu_game_dll_base = GetModuleBaseAddress(L"osu.Game.dll");
    if (!osu_game_dll_base)
        FR_ERROR("GetModuleBaseAddress osu.Game.dll");
    init_difficulty(osu_game_dll_base);
    patch_osu_game_dll(osu_game_dll_base);
    scan_for_code_starts();
    init_on_beatmap_changed();
}
