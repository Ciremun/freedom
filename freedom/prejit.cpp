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
constexpr int32_t classmethod_types_count = (int32_t)ClassMethodType::COUNT;

std::string cm_load_s;
std::string cm_replay_s;
std::string cm_score_s;
std::string cm_checkflashlight_s;
std::string cm_updateflashlight_s;
std::string cm_checktime_s;
std::string cm_updatevariables_s;

static inline std::string get_utf8(const std::wstring &wstr)
{
    if (wstr.empty()) return std::string();
    int sz = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], -1, 0, 0, 0, 0);
    std::string res(sz, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], -1, &res[0], sz, 0, 0);
    return res;
}

static inline std::wstring get_utf16(const std::string &str)
{
    if (str.empty()) return std::wstring();
    int sz = MultiByteToWideChar(CP_UTF8, 0, &str[0], -1, 0, 0);
    std::wstring res(sz, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], -1, &res[0], sz);
    return res;
}

static inline std::string get_utf8_for_classmethod(wchar_t *class_, wchar_t *method, ClassMethodType type)
{
    std::wstring colon_colon(L"::");
    std::wstring final_string = std::wstring(class_) + colon_colon +
                                std::wstring(method) + colon_colon +
                                std::to_wstring((int32_t)type);
    std::string ret = get_utf8(final_string);
    return ret;
}

HRESULT ExecuteInDefaultAppDomain(ICLRRuntimeHost *p, LPCWSTR pwzAssemblyPath, LPCWSTR pwzTypeName,
                                  LPCWSTR pwzMethodName, LPCWSTR pwzArgument, DWORD *pReturnValue)
{
    HRESULT r = p->ExecuteInDefaultAppDomain(pwzAssemblyPath, pwzTypeName, pwzMethodName, pwzArgument, pReturnValue);
    if (r != S_OK)
    {
        std::string type_name = get_utf8(pwzTypeName);
        std::string method_name = get_utf8(pwzMethodName);
        std::string argument = get_utf8(pwzArgument);
        if (argument.length())
            FR_ERROR_FMT("%s.%s call with arg '%s' failed, error code: 0x%X", type_name.c_str(), method_name.c_str(), argument.c_str(), r);
        else
            FR_ERROR_FMT("%s.%s call failed, error code: 0x%X", type_name.c_str(), method_name.c_str(), r);
    }
    return r;
}

void save_classmethods_from_addrs()
{
    clr_do([](ICLRRuntimeHost *p)
    {
        DWORD ClassMethodsFromAddrsPtr;
        HRESULT result = ExecuteInDefaultAppDomain(p, clr_module_path, L"Freedom.PreJit", L"GetClassMethodsFromAddrsPtr", L"", &ClassMethodsFromAddrsPtr);
        if (result == S_OK)
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
            cm_load_s =             get_utf8_for_classmethod(classmethods[0].class_, classmethods[0].method, classmethods[0].type);
            cm_replay_s =           get_utf8_for_classmethod(classmethods[1].class_, classmethods[1].method, classmethods[1].type);
            cm_score_s =            get_utf8_for_classmethod(classmethods[2].class_, classmethods[2].method, classmethods[2].type);
            cm_checkflashlight_s =  get_utf8_for_classmethod(classmethods[3].class_, classmethods[3].method, classmethods[3].type);
            cm_updateflashlight_s = get_utf8_for_classmethod(classmethods[4].class_, classmethods[4].method, classmethods[4].type);
            cm_checktime_s =        get_utf8_for_classmethod(classmethods[5].class_, classmethods[5].method, classmethods[5].type);
            cm_updatevariables_s =  get_utf8_for_classmethod(classmethods[6].class_, classmethods[6].method, classmethods[6].type);
            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        }
    });
}

void load_classmethods_from_addrs()
{
    clr_do([](ICLRRuntimeHost *p)
    {
        DWORD r;
        std::wstring cm_load_ws = get_utf16(cm_load_s);
        std::wstring cm_replay_ws = get_utf16(cm_replay_s);
        std::wstring cm_score_ws = get_utf16(cm_score_s);
        std::wstring cm_checkflashlight_ws = get_utf16(cm_checkflashlight_s);
        std::wstring cm_updateflashlight_ws = get_utf16(cm_updateflashlight_s);
        std::wstring cm_checktime_ws = get_utf16(cm_checktime_s);
        std::wstring cm_updatevariables_ws = get_utf16(cm_updatevariables_s);
        ExecuteInDefaultAppDomain(p, clr_module_path, L"Freedom.PreJit", L"SetClassMethod", cm_load_ws.c_str(), &r);
        ExecuteInDefaultAppDomain(p, clr_module_path, L"Freedom.PreJit", L"SetClassMethod", cm_replay_ws.c_str(), &r);
        ExecuteInDefaultAppDomain(p, clr_module_path, L"Freedom.PreJit", L"SetClassMethod", cm_score_ws.c_str(), &r);
        ExecuteInDefaultAppDomain(p, clr_module_path, L"Freedom.PreJit", L"SetClassMethod", cm_checkflashlight_ws.c_str(), &r);
        ExecuteInDefaultAppDomain(p, clr_module_path, L"Freedom.PreJit", L"SetClassMethod", cm_updateflashlight_ws.c_str(), &r);
        ExecuteInDefaultAppDomain(p, clr_module_path, L"Freedom.PreJit", L"SetClassMethod", cm_checktime_ws.c_str(), &r);
        ExecuteInDefaultAppDomain(p, clr_module_path, L"Freedom.PreJit", L"SetClassMethod", cm_updatevariables_ws.c_str(), &r);
        ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
    });
}

bool prejit_all()
{
    prepared_all_methods = true;

    if (clr_module_path[0] == '\0')
        return false;

    clr_do([](ICLRRuntimeHost *p)
    {
        DWORD dwRet = 0;
        ExecuteInDefaultAppDomain(p, clr_module_path, L"Freedom.PreJit", L"prejit_all", L"", &dwRet);
    });

    return true;
}

bool prejit_all_f()
{
    load_classmethods_from_addrs();

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
                ExecuteInDefaultAppDomain(pClrRuntimeHost, clr_module_path, L"Freedom.PreJit", L"prejit_all_f", L"", &dwRet);
                pClrRuntimeHost->Stop();
                pClrRuntimeHost->Release();
            }
            pRuntimeInfo->Release();
        }
        pMetaHost->Release();
    }

    return dwRet == 1;
}
