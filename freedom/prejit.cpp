// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "prejit.h"

bool prejit_all()
{
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

                wchar_t module_path[MAX_PATH * 2];
                DWORD module_path_length = GetModuleFileNameW(g_module, module_path, MAX_PATH * 2);
                if (module_path_length == 0)
                    return false;

                DWORD backslash_index = module_path_length - 1;
                while (backslash_index)
                    if (module_path[--backslash_index] == '\\')
                        break;

                memcpy(module_path + backslash_index + 1, L"prejit.dll", 10 * sizeof(WCHAR) + 1);

                HRESULT result = pClrRuntimeHost->ExecuteInDefaultAppDomain(module_path, L"Freedom.PreJit", L"prejit_all", L"", &dwRet);
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
