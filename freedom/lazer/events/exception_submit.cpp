#include "lazer/events/exception_submit.h"

bool init_on_exception_submit(uintptr_t base)
{
    if (!base)
        return false;

    // NOTE(Ciremun): shouldSubmitException Token
    uint32_t *oldRVA = (uint32_t *)token_to_rva(base, 0x0600030C);
    if (!oldRVA)
    {
        FR_ERROR("on_exception_submit: token_to_rva failed");
        return false;
    }

    FR_INFO("on_exception_submit base: %" PRIXPTR, base);
    FR_INFO("on_exception_submit old RVA: 0x%08" PRIX32, *oldRVA);
    FR_INFO("on_exception_submit code size: %" PRIXPTR, base + *oldRVA + 0x4);
    FR_INFO("on_exception_submit instructions: %" PRIXPTR, base + *oldRVA + 0xC);

    uint32_t code_size = 0x2;
    if (!internal_memory_patch((uint8_t *)(base + *oldRVA + 0x4), &code_size, sizeof(code_size)))
    {
        FR_ERROR("on_exception_submit: code size patch failed");
        return false;
    }

    uint8_t return_false[] = { 0x17, 0x2A };
    if (!internal_memory_patch((uint8_t *)(base + *oldRVA + 0xC), return_false, sizeof(return_false)))
    {
        FR_ERROR("on_exception_submit: instructions patch failed");
        return false;
    }

    return true;
}
