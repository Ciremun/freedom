// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "dotnet/dotnet_data_collector.h"

bool prejit_all()
{
    ICLRMetaHost *pMetaHost;
    ICLRRuntimeInfo *pRuntimeInfo;
    ICLRRuntimeHost *pClrRuntimeHost;
    IEnumUnknown *RuntimeEnum;
    HANDLE ths;
    MODULEENTRY32 m;
    HRESULT r;
    BOOL result = FALSE;
    WCHAR dotnetcorepath[MAX_PATH];

    CLR_DEBUGGING_VERSION v;
    v.wStructVersion = 0;
    v.wMajor = 4;
    v.wMinor = 0;
    v.wRevision = 30319;
    v.wBuild = 65535;

    ICLRDebugging *CLRDebugging = NULL;
    ICLRDebugging *CLRDebuggingCore = NULL;
    ICorDebugProcess *CorDebugProcess;
    ICorDebugProcess5 *CorDebugProcess5;

    CMyICLRDebuggingLibraryProvider *libprovider;
    CMyIcorDebugDataTarget *datacallback;

    DWORD processid = GetCurrentProcessId();
    if (processid == 0)
        return false;

    HANDLE processhandle = GetCurrentProcess();
    if (processhandle == 0)
        return false;

    HMODULE hMscoree = GetModuleHandleA("mscoree.dll");
    if (hMscoree == 0)
        return false;

    CLRCreateInstanceFnPtr CLRCreateInstance = NULL, CLRCreateInstanceDotNetCore = NULL;

    StrCpyW(dotnetcorepath, L"");

    CLRCreateInstance = (CLRCreateInstanceFnPtr)GetProcAddress(hMscoree, "CLRCreateInstance");

    if (CLRCreateInstance)
    {
        if (CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (LPVOID *)&pMetaHost) == S_OK)
        {
            if (pMetaHost->EnumerateLoadedRuntimes(processhandle, &RuntimeEnum) == S_OK)
            {
                ICLRRuntimeInfo *info;
                ULONG count = 0;
                RuntimeEnum->Next(1, (IUnknown **)&info, &count);
                if (count)
                {
                    result = TRUE;
                    libprovider = new CMyICLRDebuggingLibraryProvider(info, dotnetcorepath);
                }

                RuntimeEnum->Release();
            }
            pMetaHost->Release();
        }
    }

    if (libprovider == NULL)
        libprovider = new CMyICLRDebuggingLibraryProvider(NULL, dotnetcorepath);

    if (!result)
        return false;

    if (CLRCreateInstance)
        CLRCreateInstance(CLSID_CLRDebugging, IID_ICLRDebugging, (void **)&CLRDebugging);

    if (CLRCreateInstanceDotNetCore)
        CLRCreateInstanceDotNetCore(CLSID_CLRDebugging, IID_ICLRDebugging, (void **)&CLRDebuggingCore);

    ICLRMetaHost *pMetaHost_2;
    CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (void **)&pMetaHost_2);

    if (pMetaHost_2->GetRuntime(L"v4.0.30319", IID_ICLRRuntimeInfo, (void **)&pRuntimeInfo) == S_OK)
    {
        if (pRuntimeInfo->GetInterface(CLSID_CLRRuntimeHost, IID_ICLRRuntimeHost, (void **)&pClrRuntimeHost) == S_OK)
        {
            pClrRuntimeHost->Start();
            DWORD dwRet = 0;

            wchar_t module_path[MAX_PATH * 2];
            DWORD module_path_length = GetModuleFileNameW(g_module, module_path, MAX_PATH * 2);
            if (module_path_length == 0)
                return false;

            DWORD backslash_index = module_path_length - 1;
            while (backslash_index)
                if (module_path[--backslash_index] == '\\')
                    break;

            memcpy(module_path + backslash_index + 1, L"prejit.dll", 10 * sizeof(WCHAR) + 1);

            FR_INFO_FMT("prejit.dll path: %S", module_path);

            HRESULT result = pClrRuntimeHost->ExecuteInDefaultAppDomain(module_path, L"Freedom.PreJit", L"prejit_all", L"", &dwRet);
            if (result != S_OK)
                FR_ERROR_FMT("pClrRuntimeHost->ExecuteInDefaultAppDomain failed, error code: 0x%X", result);
            pClrRuntimeHost->Stop();
            pClrRuntimeHost->Release();
        }
    }
    return true;
}
