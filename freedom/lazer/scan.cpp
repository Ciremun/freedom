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

static bool patch_rva_boundcheck()
{
    uintptr_t coreclr_dll_base = GetModuleBaseAddress(L"coreclr.dll");
    if (!coreclr_dll_base)
    {
        FR_ERROR("GetModuleBaseAddress coreclr.dll");
        return false;
    }
    _MEMORY_BASIC_INFORMATION mbi;
    for (uint8_t *p = (uint8_t *)(coreclr_dll_base + 0x30000); VirtualQuery(p, &mbi, sizeof(mbi)) && p < (uint8_t *)(coreclr_dll_base + 0x3FFFF); p += mbi.RegionSize)
    {
        if (mbi.State != MEM_COMMIT || mbi.Protect != PAGE_EXECUTE_READ)
            continue;

        for (SIZE_T idx = 0; idx != mbi.RegionSize; ++idx)
        {
            uint8_t *opcodes = (uint8_t *)((uintptr_t)mbi.BaseAddress + idx);
            constexpr auto cmp_rax { pattern::build<"48 83 38 00"> };
            constexpr auto test_rdi { pattern::build<"F6 47 14 01"> };
            if (pattern::find<test_rdi>({ opcodes, test_rdi.size() }))
            {
                for (int cmp_rax_idx = 6; cmp_rax_idx < 32; ++cmp_rax_idx)
                {
                    if (pattern::find<cmp_rax>({ opcodes - cmp_rax_idx, cmp_rax.size() }))
                    {
                        BYTE nop[] = { 0x90, 0x90 };
                        FR_INFO("RVA boundcheck: found at coreclr.dll + 0x%08" PRIXPTR, (uintptr_t)(opcodes - cmp_rax_idx + cmp_rax.size() - coreclr_dll_base));
                        return internal_memory_patch(opcodes - cmp_rax_idx + cmp_rax.size(), nop, sizeof(nop));
                    }
                }
            }
        }
    }
    FR_ERROR("RVA boundcheck: scan failed");
    return false;
}

static void scan_for_code_starts()
{
    SIZE_T alignment = 16;
    _MEMORY_BASIC_INFORMATION mbi;
    for (uint8_t *p = (uint8_t *)0x700000000000; VirtualQuery(p, &mbi, sizeof(mbi)); p += mbi.RegionSize)
    {
        if (mbi.State != MEM_COMMIT || mbi.Protect != PAGE_EXECUTE_READ)
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

// NOTE(Ciremun): Breaks tiered compilation
static inline void patch_osu_game_dll(uintptr_t base)
{
    if (!base)
        return;
    // TODO(Ciremun): offsets header
    // TODO(Ciremun): RVA Offset
    BYTE ff = (BYTE)0xFF;
    if (!internal_memory_patch((BYTE *)(base + 0xC36C), &ff, sizeof(BYTE)))
        FR_ERROR("Failed to patch osu.Game.dll");
}

void init_hooks()
{
    uintptr_t osu_game_dll_base = GetModuleBaseAddress(L"osu.Game.dll");
    if (!osu_game_dll_base)
        FR_ERROR("GetModuleBaseAddress osu.Game.dll");

    // NOTE(Ciremun): IL patches
    // patch_osu_game_dll(osu_game_dll_base);
    if (patch_rva_boundcheck())
    {
        if (!init_difficulty(osu_game_dll_base))
            FR_ERROR("Init Difficulty failed");
    }
    else
        FR_ERROR("Failed to patch RVA boundcheck");

    // NOTE(Ciremun): Hooks
    // scan_for_code_starts();
    // init_on_beatmap_changed();
}
