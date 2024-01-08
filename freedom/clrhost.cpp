#include "clrhost.h"
#include "baked_utils_dll.h"

#import "mscorlib.tlb" auto_rename
using namespace mscorlib;

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

bool prepared_all_methods = false;
bool prepared_methods = false;
_AssemblyPtr assembly_ptr = 0;

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
    if (!class_ || !method)
        return get_utf8(colon_colon);

    std::wstring class_method_type = std::wstring(class_) + colon_colon +
                                std::wstring(method) + colon_colon +
                                std::to_wstring((int32_t)type);
    return get_utf8(class_method_type);
}

void get_classmethods_from_addrs()
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
    VARIANT variant;
    VariantInit(&variant);
    variant.vt = VT_I4;
    variant.intVal = (INT)classmethods;
    SAFEARRAY* params = SafeArrayCreateVector(VT_VARIANT, 0, 2);
    LONG i = 0;
    SafeArrayPutElement(params, &i, &variant);
    ++i;
    variant.vt = VT_I4;
    variant.intVal = classmethod_types_count;
    SafeArrayPutElement(params, &i, &variant);
    VARIANT v = invoke_csharp_method(L"Freedom.Utils", L"ClassMethodsFromAddrs", params);
    if (variant_ok(&v) && v.intVal == 1)
    {
        cm_load_s =             get_utf8_for_classmethod(classmethods[0].class_, classmethods[0].method, classmethods[0].type);
        cm_replay_s =           get_utf8_for_classmethod(classmethods[1].class_, classmethods[1].method, classmethods[1].type);
        cm_score_s =            get_utf8_for_classmethod(classmethods[2].class_, classmethods[2].method, classmethods[2].type);
        cm_checkflashlight_s =  get_utf8_for_classmethod(classmethods[3].class_, classmethods[3].method, classmethods[3].type);
        cm_updateflashlight_s = get_utf8_for_classmethod(classmethods[4].class_, classmethods[4].method, classmethods[4].type);
        cm_checktime_s =        get_utf8_for_classmethod(classmethods[5].class_, classmethods[5].method, classmethods[5].type);
        cm_updatevariables_s =  get_utf8_for_classmethod(classmethods[6].class_, classmethods[6].method, classmethods[6].type);
        ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
    }
    SafeArrayDestroy(params);
}

static inline bool set_classmethods_from_strings()
{
    VARIANT v;
    std::wstring cm_load_ws = get_utf16(cm_load_s);
    std::wstring cm_replay_ws = get_utf16(cm_replay_s);
    std::wstring cm_score_ws = get_utf16(cm_score_s);
    std::wstring cm_checkflashlight_ws = get_utf16(cm_checkflashlight_s);
    std::wstring cm_updateflashlight_ws = get_utf16(cm_updateflashlight_s);
    std::wstring cm_checktime_ws = get_utf16(cm_checktime_s);
    std::wstring cm_updatevariables_ws = get_utf16(cm_updatevariables_s);
#define RETURN_IF_FALSE() if (variant_ok(&v) && v.intVal != 1) return false
    v = invoke_csharp_method(L"Freedom.Utils", L"SetClassMethod", cm_load_ws.c_str());             RETURN_IF_FALSE();
    v = invoke_csharp_method(L"Freedom.Utils", L"SetClassMethod", cm_replay_ws.c_str());           RETURN_IF_FALSE();
    v = invoke_csharp_method(L"Freedom.Utils", L"SetClassMethod", cm_score_ws.c_str());            RETURN_IF_FALSE();
    v = invoke_csharp_method(L"Freedom.Utils", L"SetClassMethod", cm_checkflashlight_ws.c_str());  RETURN_IF_FALSE();
    v = invoke_csharp_method(L"Freedom.Utils", L"SetClassMethod", cm_updateflashlight_ws.c_str()); RETURN_IF_FALSE();
    v = invoke_csharp_method(L"Freedom.Utils", L"SetClassMethod", cm_checktime_ws.c_str());        RETURN_IF_FALSE();
    v = invoke_csharp_method(L"Freedom.Utils", L"SetClassMethod", cm_updatevariables_ws.c_str());  RETURN_IF_FALSE();
#undef RETURN_IF_FALSE
    ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
    FR_INFO("[+] All classmethods are set");
    return true;
}

bool prepare_all_methods_fast()
{
    if (!set_classmethods_from_strings())
    {
        FR_INFO("[!] Failed to set some classmethods, fallback to slow method");
        return false;
    }
    VARIANT v = invoke_csharp_method(L"Freedom.Utils", L"prepare_all_methods_fast");
    if (variant_ok(&v) && v.intVal == 1)
    {
        prepared_methods = true;
        return true;
    }
    return false;
}

bool prepare_all_methods_slow()
{
    double s = ImGui::GetTime();
    invoke_csharp_method(L"Freedom.Utils", L"prepare_all_methods_slow");
    FR_INFO_FMT("Preparing All Methods Slow Took: %lfs", ImGui::GetTime() - s);
    prepared_all_methods = true;
    prepared_methods = true;
    return true;
}

static inline ICorRuntimeHost* getCorRtHost_byVersion(LPCWSTR sz_runtimeVersion) {
	ICLRRuntimeInfo* pRuntimeInfo = NULL;
	ICorRuntimeHost* pRuntimeHost = NULL;
	ICLRMetaHost* pMetaHost = NULL;
	BOOL bLoadable;

    HMODULE hMscoree = GetModuleHandleA("mscoree.dll");
    if (hMscoree == 0)
        return NULL;

    CLRCreateInstanceFnPtr CLRCreateInstance = (CLRCreateInstanceFnPtr)GetProcAddress(hMscoree, "CLRCreateInstance");
    if (!CLRCreateInstance)
        return NULL;

	if (FAILED(CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (VOID**)&pMetaHost)))
	{
		FR_INFO("[!] CLRCreateInstance");
		return NULL;
	}
	else FR_INFO("[+] CLRCreateInstance");

	if (FAILED(pMetaHost->GetRuntime(sz_runtimeVersion, IID_ICLRRuntimeInfo, (VOID**)&pRuntimeInfo))) {
		FR_INFO_FMT("[!] GetRuntime failed: %S", sz_runtimeVersion);
		return NULL;
	}
	else FR_INFO("[+] GetRuntime");

	if (FAILED(pRuntimeInfo->IsLoadable(&bLoadable)) || !bLoadable) {
		FR_INFO("[!] IsLoadable");
		return NULL;
	}
	else FR_INFO("[+] IsLoadable");

	if (FAILED(pRuntimeInfo->GetInterface(CLSID_CorRuntimeHost, IID_ICorRuntimeHost, (VOID**)&pRuntimeHost))) {
		FR_INFO("[!] GetInterface");
		return NULL;
	}
	else FR_INFO("[+] GetInterface");

	if (FAILED(pRuntimeHost->Start())) {
		FR_INFO("[!] Start");
		return NULL;
	}
	else FR_INFO("[+] Start");
	return pRuntimeHost;
}

static inline _AppDomainPtr getDefaultDomain(ICorRuntimeHost* pRuntimeHost) {
	IUnknownPtr pAppDomainThunk = NULL;
	if (FAILED(pRuntimeHost->GetDefaultDomain(&pAppDomainThunk))) {
		FR_INFO("[!] GetDefaultDomain");
		return NULL;
	}
	else FR_INFO("[+] GetDefaultDomain");

	_AppDomainPtr pDefaultAppDomain = NULL;
	if (FAILED(pAppDomainThunk->QueryInterface(__uuidof(_AppDomain), (LPVOID*)&pDefaultAppDomain))) {
		FR_INFO("[!] QueryInterface");
		return NULL;
	}
	else FR_INFO("[+] QueryInterface");
	return pDefaultAppDomain;
}

static inline _AssemblyPtr getAssembly_fromBinary(_AppDomainPtr pDefaultAppDomain, LPBYTE rawData, ULONG lenRawData) {
	_AssemblyPtr pAssembly = NULL;
	SAFEARRAY* pSafeArray = SafeArrayCreate(VT_UI1, 1, new SAFEARRAYBOUND{ lenRawData , 0 });

	void* pvData = NULL;
	if (FAILED(SafeArrayAccessData(pSafeArray, &pvData))) {
		FR_INFO("[!] SafeArrayAccessData");
		return NULL;
	}
	else FR_INFO("[+] SafeArrayAccessData");

	memcpy(pvData, rawData, lenRawData);
	if (FAILED(SafeArrayUnaccessData(pSafeArray))) {
		FR_INFO("[!] SafeArrayUnaccessData");
		return NULL;
	}
	else FR_INFO("[+] SafeArrayUnaccessData");

	if (pAssembly = pDefaultAppDomain->Load_3(pSafeArray))
        FR_INFO("[+] Load");
	else
    {
		FR_INFO("[!] Load");
        return NULL;
    }
    if (FAILED(SafeArrayDestroy(pSafeArray))) {
		FR_INFO("[!] SafeArrayDestroy");
		return pAssembly;
	}
	else FR_INFO("[+] SafeArrayDestroy");

	return pAssembly;
}

bool load_csharp_assembly()
{
	FR_INFO(" --- Try to Fetch .NET Framework v4.0.30319 ---");
	ICorRuntimeHost* pRuntimeHost = getCorRtHost_byVersion(L"v4.0.30319");

	if (!pRuntimeHost)
		return false;

	FR_INFO(" --- Execute .NET Module ---");
	_MethodInfoPtr pMethodInfo = NULL;
	if (auto pDefaultAppDomain = getDefaultDomain(pRuntimeHost))
    {
		if (assembly_ptr = getAssembly_fromBinary(pDefaultAppDomain, LPBYTE(utils_dll_data), utils_dll_size))
        {
            FR_INFO("--- Done! ---");
            return true;
        }
        FR_INFO("[!] --- Get Assembly From Binary Failed ---");
        return false;
    }
    FR_INFO("[!] --- Get Default Domain Failed ---");
    return false;
}

bool variant_ok(void *variant_ptr)
{
    VARIANT *v = (VARIANT *)variant_ptr;
    return v->vt != VT_EMPTY;
}

VARIANT invoke_csharp_method(const wchar_t *type_name, const wchar_t *method_name, const wchar_t *wchar_string_arg)
{
    VARIANT variant;
    VariantInit(&variant);
    variant.vt = VT_BSTR;
    variant.bstrVal = SysAllocString(wchar_string_arg);
    SAFEARRAY* params = SafeArrayCreateVector(VT_VARIANT, 0, 1);
    LONG i = 0;
    SafeArrayPutElement(params, &i, &variant);
    VARIANT v = invoke_csharp_method(type_name, method_name, params);
    SafeArrayDestroy(params);
    SysFreeString(variant.bstrVal);
    return v;
}

VARIANT invoke_csharp_method(const wchar_t *type_name, const wchar_t *method_name)
{
    VARIANT variant;
    VariantInit(&variant);
    SAFEARRAY* params = SafeArrayCreateVector(VT_VARIANT, 0, 0);
    VARIANT v = invoke_csharp_method(type_name, method_name, params);
    SafeArrayDestroy(params);
    return v;
}

VARIANT invoke_csharp_method(const wchar_t *type_name, const wchar_t *method_name, SAFEARRAY* params)
{
    VARIANT variant;
    VariantInit(&variant);

    if (assembly_ptr == 0)
    {
        FR_INFO("[!] invoke_csharp_method failed, assembly_ptr is null");
        return variant;
    }

    std::string type_name_s = get_utf8(type_name);
    std::string method_name_s = get_utf8(method_name);

    _TypePtr type_ptr;
    BSTR type_name_b = SysAllocString(type_name);
    if (type_ptr = assembly_ptr->GetType_2(type_name_b))
    { /* FR_INFO("[+] GetType"); */ }
	else
    {
        FR_INFO_FMT("[!] Invoke %s::%s", type_name_s.c_str(), method_name_s.c_str());
        FR_INFO("[!] GetType");
        SysFreeString(type_name_b);
		return variant;
    }

    _MethodInfoPtr method_ptr;
    BSTR method_name_b = SysAllocString(method_name);
    if (method_ptr = type_ptr->GetMethod_2(method_name_b, (BindingFlags)(BindingFlags_Public | BindingFlags_Static)))
    { /* FR_INFO("[+] GetMethod"); */ }
	else
    {
        FR_INFO_FMT("[!] Invoke %s::%s", type_name_s.c_str(), method_name_s.c_str());
        FR_INFO("[!] GetMethod");
        SysFreeString(type_name_b);
        SysFreeString(method_name_b);
		return variant;
    }

    HRESULT hr = method_ptr->raw_Invoke_3(variant, params, &variant);
    if (FAILED(hr))
    {
        FR_INFO_FMT("[!] Invoke %s::%s", type_name_s.c_str(), method_name_s.c_str());
        FR_INFO_FMT("[!] Invoke (0x%X)", hr);
    }

    SysFreeString(type_name_b);
    SysFreeString(method_name_b);

    FR_INFO_FMT("[+] Invoke %s::%s", type_name_s.c_str(), method_name_s.c_str());

    return variant;
}
