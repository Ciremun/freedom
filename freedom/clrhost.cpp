#include "clrhost.h"
#include "baked_utils_dll.h"

#import "mscorlib.tlb" auto_rename
using namespace mscorlib;

int prepared_methods_count = -1;
_AssemblyPtr assembly_ptr = 0;

static inline std::string get_utf8(const std::wstring &wstr)
{
    if (wstr.empty()) return std::string();
    int sz = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], -1, 0, 0, 0, 0);
    std::string res(sz, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], -1, &res[0], sz, 0, 0);
    return res;
}

bool prepare_methods()
{
    double s = ImGui::GetTime();
    VARIANT v = invoke_csharp_method(L"Freedom.Utils", L"PrepareMethods");
    if (variant_ok(v))
        prepared_methods_count = V_INT(&v);
    FR_INFO_FMT("Preparing Methods Took: %lfs", ImGui::GetTime() - s);
    return true;
}

bool variant_ok(VARIANT variant)
{
    return V_VT(&variant) != VT_EMPTY;
}

static inline ICorRuntimeHost* init_clr_runtime_host(LPCWSTR sz_runtimeVersion) {
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

	if (FAILED(pMetaHost->GetRuntime(sz_runtimeVersion, IID_ICLRRuntimeInfo, (VOID**)&pRuntimeInfo))) {
		FR_INFO_FMT("[!] GetRuntime failed: %S", sz_runtimeVersion);
		return NULL;
	}

	if (FAILED(pRuntimeInfo->IsLoadable(&bLoadable)) || !bLoadable) {
		FR_INFO("[!] IsLoadable");
		return NULL;
	}

	if (FAILED(pRuntimeInfo->GetInterface(CLSID_CorRuntimeHost, IID_ICorRuntimeHost, (VOID**)&pRuntimeHost))) {
		FR_INFO("[!] GetInterface");
		return NULL;
	}

	if (FAILED(pRuntimeHost->Start())) {
		FR_INFO("[!] Start");
		return NULL;
	}
	return pRuntimeHost;
}

static inline _AppDomainPtr get_default_domain(ICorRuntimeHost* pRuntimeHost) {
	IUnknownPtr pAppDomainThunk = NULL;
	if (FAILED(pRuntimeHost->GetDefaultDomain(&pAppDomainThunk))) {
		FR_INFO("[!] GetDefaultDomain");
		return NULL;
	}
	_AppDomainPtr pDefaultAppDomain = NULL;
	if (FAILED(pAppDomainThunk->QueryInterface(__uuidof(_AppDomain), (LPVOID*)&pDefaultAppDomain))) {
		FR_INFO("[!] QueryInterface");
		return NULL;
	}
	return pDefaultAppDomain;
}

static inline _AssemblyPtr get_assembly_from_binary(_AppDomainPtr pDefaultAppDomain, LPBYTE rawData, ULONG lenRawData) {
	_AssemblyPtr pAssembly = NULL;
	SAFEARRAY* pSafeArray = SafeArrayCreate(VT_UI1, 1, new SAFEARRAYBOUND{ lenRawData , 0 });

	void* pvData = NULL;
	if (FAILED(SafeArrayAccessData(pSafeArray, &pvData))) {
		FR_INFO("[!] SafeArrayAccessData");
		return NULL;
	}

	memcpy(pvData, rawData, lenRawData);
	if (FAILED(SafeArrayUnaccessData(pSafeArray))) {
		FR_INFO("[!] SafeArrayUnaccessData");
		return NULL;
	}

	pAssembly = pDefaultAppDomain->Load_3(pSafeArray);
    if (!pAssembly) {
		FR_INFO("[!] Load");
        return NULL;
    }
    if (FAILED(SafeArrayDestroy(pSafeArray))) {
		FR_INFO("[!] SafeArrayDestroy");
		return pAssembly;
	}

	return pAssembly;
}

bool init_clrhost()
{
	ICorRuntimeHost* pRuntimeHost = init_clr_runtime_host(L"v4.0.30319");

	if (!pRuntimeHost)
		return false;

	_MethodInfoPtr pMethodInfo = NULL;
	if (auto pDefaultAppDomain = get_default_domain(pRuntimeHost))
    {
		if (assembly_ptr = get_assembly_from_binary(pDefaultAppDomain, LPBYTE(utils_dll_data), utils_dll_size))
            return true;
        FR_INFO("[!] C# Get Assembly From Binary Failed");
        return false;
    }
    FR_INFO("[!] C# Get Default Domain Failed");
    return false;
}

VARIANT invoke_csharp_method(const wchar_t *type_name, const wchar_t *method_name, const wchar_t *wchar_string_arg)
{
    VARIANT variant;
    VariantInit(&variant);
    V_VT(&variant) = VT_BSTR;
    V_BSTR(&variant) = SysAllocString(wchar_string_arg);
    SAFEARRAY* params = SafeArrayCreateVector(VT_VARIANT, 0, 1);
    LONG i = 0;
    SafeArrayPutElement(params, &i, &variant);
    VARIANT v = invoke_csharp_method(type_name, method_name, params);
    SafeArrayDestroy(params);
    SysFreeString(V_BSTR(&variant));
    return v;
}

VARIANT invoke_csharp_method(const wchar_t *type_name, const wchar_t *method_name)
{
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

    FR_INFO_FMT("Invoke %s::%s", type_name_s.c_str(), method_name_s.c_str());

    _TypePtr type_ptr = 0;
    BSTR type_name_b = SysAllocString(type_name);
    HRESULT hr = assembly_ptr->raw_GetType_2(type_name_b, &type_ptr);
    if (type_ptr)
    {
        // FR_INFO("[+] GetType");
    }
	else
    {
        FR_INFO_FMT("[!] GetType (0x%X)", hr);
        SysFreeString(type_name_b);
		return variant;
    }

    _MethodInfoPtr method_ptr = 0;
    BSTR method_name_b = SysAllocString(method_name);
    hr = type_ptr->raw_GetMethod_2(method_name_b, (BindingFlags)(BindingFlags_Public | BindingFlags_Static), &method_ptr);
    if (method_ptr)
    {
        // FR_INFO("[+] GetMethod");
    }
	else
    {
        FR_INFO_FMT("[!] GetMethod (0x%X)", hr);
        SysFreeString(type_name_b);
        SysFreeString(method_name_b);
		return variant;
    }

    hr = method_ptr->raw_Invoke_3(variant, params, &variant);
    if (FAILED(hr))
    {
        FR_INFO_FMT("[!] Invoke (0x%X)", hr);
        SysFreeString(type_name_b);
        SysFreeString(method_name_b);
        return variant;
    }

    SysFreeString(type_name_b);
    SysFreeString(method_name_b);

    return variant;
}
