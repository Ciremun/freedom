#pragma once

#include "stdafx.h"

#include "mem.h"

#include <vector>
#include <string>

#include <stdint.h>

#include "features/relax.h"
#include "features/aimbot.h"
#include "features/difficulty.h"
#include "features/discord_rpc.h"
#include "features/replay.h"
#include "features/score_multiplier.h"
#include "features/unmod_flashlight.h"
#include "features/timewarp.h"
#include "features/hidden_remover.h"

extern wchar_t clr_module_path[MAX_PATH * 2];
extern bool prepared_all_methods;

bool prejit_all();
bool prejit_all_f();
void save_classmethods_from_addrs();
HRESULT ExecuteInDefaultAppDomain(ICLRRuntimeHost *p, LPCWSTR pwzAssemblyPath, LPCWSTR pwzTypeName,
                                  LPCWSTR pwzMethodName, LPCWSTR pwzArgument, DWORD *pReturnValue);

template <typename T>
BOOL clr_do(T callback)
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
                callback(pClrRuntimeHost);
                pClrRuntimeHost->Stop();
                pClrRuntimeHost->Release();
            }
            pRuntimeInfo->Release();
        }
        pMetaHost->Release();
    }
    return true;
}
