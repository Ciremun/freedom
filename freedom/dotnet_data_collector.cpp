#include "dotnet_data_collector.h"

uintptr_t code_start_for_parse_beatmap_metadata()
{
    ICLRMetaHost *pMetaHost;
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
        return 0;

    HANDLE processhandle = GetCurrentProcess();
    if (processhandle == 0)
        return 0;

    HMODULE hMscoree = GetModuleHandleA("mscoree.dll");
    if (hMscoree == 0)
        return 0;

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
        return 0;

    if (CLRCreateInstance)
        CLRCreateInstance(CLSID_CLRDebugging, IID_ICLRDebugging, (void **)&CLRDebugging);

    if (CLRCreateInstanceDotNetCore)
        CLRCreateInstanceDotNetCore(CLSID_CLRDebugging, IID_ICLRDebugging, (void **)&CLRDebuggingCore);

    ths = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processid);
    if (ths == INVALID_HANDLE_VALUE)
        return 0;

    ZeroMemory(&m, sizeof(m));
    m.dwSize = sizeof(m);
    if (Module32First(ths, &m))
    {
        datacallback = new CMyIcorDebugDataTarget(processhandle);
        do
        {
            CLR_DEBUGGING_PROCESS_FLAGS flags;
            if (CLRDebugging)
                r = CLRDebugging->OpenVirtualProcess((ULONG64)m.hModule, datacallback, libprovider, &v, IID_ICorDebugProcess, (IUnknown **)&CorDebugProcess, &v, &flags);

            if ((r != S_OK) && (CLRDebuggingCore))
                r = CLRDebuggingCore->OpenVirtualProcess((ULONG64)m.hModule, datacallback, libprovider, &v, IID_ICorDebugProcess, (IUnknown **)&CorDebugProcess, &v, &flags);

            if (r == S_OK)
            {
                CorDebugProcess->QueryInterface(IID_ICorDebugProcess5, (void **)&CorDebugProcess5);
                break;
            }

        }

        while (Module32Next(ths, &m));
    }

    CloseHandle(ths);

    if (CorDebugProcess)
    {
        ICorDebugAppDomainEnum *AppDomains;
        DWORD32 NumberOfDomains = 0;

        if (CorDebugProcess->EnumerateAppDomains(&AppDomains) == S_OK)
        {
            ULONG count;
            ICorDebugAppDomain **domains;
            ULONG32 namelength = 0;
            TCHAR domainname[255];

            AppDomains->GetCount(&count);
            domains = (ICorDebugAppDomain **)malloc(count * sizeof(ICorDebugAppDomain *));
            AppDomains->Next(count, domains, &count);

            NumberOfDomains = count;
            domains[0]->GetName(255, &namelength, domainname);

            AppDomains->Release();
            ICorDebugAppDomain *domain = (ICorDebugAppDomain *)domains[0]; // osu!.exe domain
            ICorDebugAssemblyEnum *pAssemblies;
            ICorDebugAssembly *assemblies[5];
            DWORD32 NumberOfModules = 0;
            unsigned int i, j;
            count = 0;
            HRESULT r;

            std::vector<ICorDebugModule *> modulelist;

            if ((!domain) || (domain->EnumerateAssemblies(&pAssemblies) != S_OK))
                return 0;

            if (domains)
                free(domains);

            do
            {
                r = pAssemblies->Next(5, assemblies, &count);
                for (i = 0; i < count; i++)
                {
                    ICorDebugModuleEnum *pModules;
                    if (assemblies[i]->EnumerateModules(&pModules) == S_OK)
                    {
                        ULONG modulecount;
                        ICorDebugModule *modules[5];
                        HRESULT r2;
                        do
                        {
                            r2 = pModules->Next(5, modules, &modulecount);
                            for (j = 0; j < modulecount; j++)
                                modulelist.push_back(modules[j]);

                        } while (r2 == S_OK);

                        pModules->Release();
                    }
                    assemblies[i]->Release();
                }
            } while (r == S_OK);

            pAssemblies->Release();

            NumberOfModules = (DWORD32)modulelist.size();
            for (i = 0; i < NumberOfModules; i++)
            {
                UINT64 hModule = (UINT64)modulelist[i];
                TCHAR modulename[255];
                ULONG32 modulenamelength;

                if (modulelist[i]->GetName(255, &modulenamelength, modulename) == S_OK)
                {
                    if (modulenamelength && wmemcmp(modulename + modulenamelength - 9, L"osu!.exe", 8) == 0)
                    {
                        ICorDebugModule *module = (ICorDebugModule *)hModule;
                        IMetaDataImport *MetaData = NULL;
                        HCORENUM henum = 0;
                        HRESULT r;
                        ULONG typedefcount;

                        mdTypeDef typedefs[256];
                        unsigned int i;

                        TCHAR typedefname[255];
                        ULONG typedefnamesize;
                        DWORD typedefflags;
                        mdToken extends;

                        if (module == NULL)
                            return 0;

                        module->GetMetaDataInterface(IID_IMetaDataImport, (IUnknown **)&MetaData);

                        if (MetaData == NULL)
                            return 0;

                        MetaData->EnumTypeDefs(&henum, typedefs, 0, &typedefcount);

                        do
                        {
                            r = MetaData->EnumTypeDefs(&henum, typedefs, 256, &typedefcount);

                            for (i = 0; i < typedefcount; i++)
                            {
                                typedefnamesize = 0;
                                MetaData->GetTypeDefProps(typedefs[i], typedefname, 255, &typedefnamesize, &typedefflags, &extends);
                                if (wmemcmp(typedefname, L"#=z9DEJsV779TWhgkKS1GefTZYVJDPgn5L2xrCk5pyAIyAH", 47) == 0)
                                {
                                    mdTypeDef TypeDef = typedefs[i];
                                    HRESULT r;
                                    unsigned int i;
                                    HCORENUM henum = 0;
                                    ULONG count = 0;
                                    mdMethodDef methods[32];
                                    TCHAR methodname[255];
                                    ULONG methodnamesize;

                                    r = MetaData->EnumMethods(&henum, TypeDef, methods, 0, &count);
                                    MetaData->CountEnum(henum, &count);

                                    do
                                    {
                                        r = MetaData->EnumMethods(&henum, TypeDef, methods, 32, &count);
                                        for (i = 0; i < count; i++)
                                        {
                                            mdTypeDef classdef;
                                            unsigned int j;
                                            DWORD dwAttr;
                                            PCCOR_SIGNATURE sig;
                                            ULONG sigsize;
                                            ULONG CodeRVA;
                                            DWORD dwImplFlags;
                                            ICorDebugFunction *df = NULL;

                                            CORDB_ADDRESS NativeCode = 0;

                                            methodnamesize = 0;
                                            MetaData->GetMethodProps(methods[i], &classdef, methodname, 255, &methodnamesize, &dwAttr, &sig, &sigsize, &CodeRVA, &dwImplFlags);

                                            methodnamesize = sizeof(TCHAR) * methodnamesize;

                                            if (wmemcmp(methodname, L"#=zyO2ZBz4=", 11) == 0)
                                            {
                                                if (module->GetFunctionFromToken(methods[i], &df) == S_OK)
                                                {
                                                    ICorDebugCode *Code;
                                                    if (df->GetNativeCode(&Code) == S_OK)
                                                    {
                                                        Code->GetAddress(&NativeCode);
                                                        Code->Release();
                                                        if (henum)
                                                            MetaData->CloseEnum(henum);
                                                        return (uintptr_t)NativeCode;
                                                    }
                                                }
                                            }
                                        }
                                    } while (r == S_OK);
                                    if (henum)
                                        MetaData->CloseEnum(henum);
                                }
                            }
                        } while (r == S_OK);
                        MetaData->CloseEnum(henum);
                    }
                }
            }
        }
    }
    return 0;
}
