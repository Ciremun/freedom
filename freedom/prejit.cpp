// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "prejit.h"

typedef void (__stdcall *ClassMethodsFromAddrsFuncPtr)(int32_t *a, int32_t size);

enum class ClassMethodType : int32_t
{
    Load = 0,
    Replay = 1,
    Score = 2,
    CheckFlashlight = 3,
    UpdateFlashlight = 4,
    CheckTime = 5,
    UpdateVariables = 6,
    COUNT = 7,
};
constexpr int32_t classmethod_types_count = (int32_t)ClassMethodType::COUNT;

#pragma pack(push)
#pragma pack(4)
struct ClassMethod
{
    wchar_t *class_ = 0;
    wchar_t *method = 0;
    ClassMethodType type = ClassMethodType::COUNT;
    uintptr_t address = 0;
    uintptr_t found = 0;
};
#pragma pack(pop)

wchar_t clr_module_path[MAX_PATH * 2] = {0};
bool prepared_all_methods = false;

void save_classmethods_from_addrs()
{
    clr_do([](ICLRRuntimeHost *p)
    {
        DWORD ClassMethodsFromAddrsPtr;
        HRESULT result = p->ExecuteInDefaultAppDomain(clr_module_path, L"Freedom.PreJit", L"GetClassMethodsFromAddrsPtr", L"", &ClassMethodsFromAddrsPtr);
        if (result != S_OK)
            FR_ERROR_FMT("pClrRuntimeHost->ExecuteInDefaultAppDomain failed, error code: 0x%X", result);
        else
        {
            ClassMethod classmethods[classmethod_types_count] = {
                {.type = ClassMethodType::Load,             .address = beatmap_onload_code_start},
                {.type = ClassMethodType::Replay,           .address = selected_replay_code_start},
                {.type = ClassMethodType::Score,            .address = score_multiplier_code_start - 0xA},
                {.type = ClassMethodType::CheckFlashlight,  .address = check_flashlight_code_start},
                {.type = ClassMethodType::UpdateFlashlight, .address = update_flashlight_code_start},
                {.type = ClassMethodType::CheckTime,        .address = check_timewarp_code_start},
                {.type = ClassMethodType::UpdateVariables,  .address = hom_update_vars_code_start},
            };
            ClassMethodsFromAddrsFuncPtr ClassMethodsFromAddrs = (ClassMethodsFromAddrsFuncPtr)(uintptr_t)ClassMethodsFromAddrsPtr;
            ClassMethodsFromAddrs((int32_t *)classmethods, classmethod_types_count);

            for (int32_t i = 0; i < classmethod_types_count; ++i)
            {
                if (classmethods[i].type == ClassMethodType::UpdateVariables)
                    printf("got classmethod: %S %S %d 0x%X\n", classmethods[i].class_, L"SOME BS METHOD", classmethods[i].type, classmethods[i].address);
                else
                    printf("got classmethod: %S %S %d 0x%X\n", classmethods[i].class_, classmethods[i].method, classmethods[i].type, classmethods[i].address);
            }
        }
    });
}

bool prejit_all()
{
    prepared_all_methods = true;

    if (clr_module_path[0] == '\0')
        return false;

    ICLRMetaHost *pMetaHost = 0;
    ICLRRuntimeInfo *pRuntimeInfo = 0;
    ICLRRuntimeHost *pClrRuntimeHost = 0;

    HMODULE hMscoree = GetModuleHandleA("mscoree.dll");
    if (hMscoree == 0)
        return false;

    CLRCreateInstanceFnPtr CLRCreateInstance = (CLRCreateInstanceFnPtr)GetProcAddress(hMscoree, "CLRCreateInstance");
    if (!CLRCreateInstance)
        return false;

    if (CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (void **)&pMetaHost) == S_OK)
    {
        if (pMetaHost->GetRuntime(L"v4.0.30319", IID_ICLRRuntimeInfo, (void **)&pRuntimeInfo) == S_OK)
        {
            if (pRuntimeInfo->GetInterface(CLSID_CLRRuntimeHost, IID_ICLRRuntimeHost, (void **)&pClrRuntimeHost) == S_OK)
            {
                pClrRuntimeHost->Start();
                DWORD dwRet = 0;

                HRESULT result = pClrRuntimeHost->ExecuteInDefaultAppDomain(clr_module_path, L"Freedom.PreJit", L"prejit_all", L"", &dwRet);
                if (result != S_OK)
                    FR_ERROR_FMT("pClrRuntimeHost->ExecuteInDefaultAppDomain failed, error code: 0x%X", result);
                pClrRuntimeHost->Stop();
                pClrRuntimeHost->Release();
            }
            pRuntimeInfo->Release();
        }
        pMetaHost->Release();
    }

    return true;
}

bool prejit_all_f()
{
    if (clr_module_path[0] == '\0')
        return false;

    ICLRMetaHost *pMetaHost = 0;
    ICLRRuntimeInfo *pRuntimeInfo = 0;
    ICLRRuntimeHost *pClrRuntimeHost = 0;

    HMODULE hMscoree = GetModuleHandleA("mscoree.dll");
    if (hMscoree == 0)
        return false;

    CLRCreateInstanceFnPtr CLRCreateInstance = (CLRCreateInstanceFnPtr)GetProcAddress(hMscoree, "CLRCreateInstance");
    if (!CLRCreateInstance)
        return false;

    DWORD dwRet = 0;

    if (CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (void **)&pMetaHost) == S_OK)
    {
        if (pMetaHost->GetRuntime(L"v4.0.30319", IID_ICLRRuntimeInfo, (void **)&pRuntimeInfo) == S_OK)
        {
            if (pRuntimeInfo->GetInterface(CLSID_CLRRuntimeHost, IID_ICLRRuntimeHost, (void **)&pClrRuntimeHost) == S_OK)
            {
                pClrRuntimeHost->Start();

                HRESULT result = pClrRuntimeHost->ExecuteInDefaultAppDomain(clr_module_path, L"Freedom.PreJit", L"prejit_all_f", L"", &dwRet);
                if (result != S_OK)
                    FR_ERROR_FMT("pClrRuntimeHost->ExecuteInDefaultAppDomain failed, error code: 0x%X", result);
                pClrRuntimeHost->Stop();
                pClrRuntimeHost->Release();
            }
            pRuntimeInfo->Release();
        }
        pMetaHost->Release();
    }

    return dwRet == 1;
}
