#define NOBUILD_IMPLEMENTATION
#include "nobuild.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "vendor/stb_sprintf.h"

#define COMMON_DIRS "freedom/", "freedom/features/", "freedom/ui/", "vendor/imgui/", "vendor/imgui/backends/"
#define DLL_DIRS "freedom/dll/", COMMON_DIRS
#define STANDALONE_DIRS "freedom/standalone/", COMMON_DIRS, "vendor/imgui/backends/standalone/"

#define MSVC_COMMON_FLAGS "/EHsc", "/nologo", "/DWIN32_LEAN_AND_MEAN", "/DUNICODE", "/std:c++latest"
#define MSVC_INCLUDE_FLAGS "/Iinclude", "/Ivendor", "/Ivendor/imgui", "/Ivendor/imgui/backends", "/Ivendor/imgui/backends/standalone", "/Ivendor/GLFW/include"

#define MSVC_RELEASE_FLAGS MSVC_COMMON_FLAGS, "/DNDEBUG", MSVC_INCLUDE_FLAGS, "/O2", "/MT", "/GL"
#define MSVC_DEBUG_FLAGS MSVC_COMMON_FLAGS, MSVC_INCLUDE_FLAGS, "/Od", "/Z7", "/MTd", "/FS"

#define MSVC_LINK_RELEASE_FLAGS "/LTCG", "/MACHINE:x86"
#define MSVC_LINK_DEBUG_FLAGS "/DEBUG", "/MACHINE:x86"

#define PROCESSES_CAPACITY 256

#define OBJS_FOR_DIRS(objs, body, ...)                                           \
    do                                                                           \
    {                                                                            \
        Cstr_Array objs = cstr_array_make(NULL);                                 \
        const char *dirs[] = {__VA_ARGS__};                                      \
        for (size_t i = 0; i < (sizeof(dirs) / sizeof(dirs[0])); ++i)            \
        {                                                                        \
            FOREACH_FILE_IN_DIR(file, dirs[i],                                   \
            {                                                                    \
                if (ENDS_WITH(file, ".cpp"))                                     \
                    objs = cstr_array_append(objs, CONCAT(NOEXT(file), ".obj")); \
            });                                                                  \
        }                                                                        \
        body;                                                                    \
    } while (0)

static inline void set_git_commit_hash();
static inline bool find_osu_exe(LPWSTR osu_exe_path);
static inline bool launch_osu(LPWSTR osu_exe_path);
static void async_obj_foreach_file_in_dirs(Cstr first, ...);

int *standalone_flag = 0;
int *debug_flag = 0;
int *rebuild_flag = 0;
int *run_flag = 0;
int *all_flag = 0;
int *console_flag = 0;

int do_link = 0;
char define_commit_hash[32] = {0};
wchar_t osu_exe_path[MAX_PATH * 2] = {0};

static void build_freedom_dll()
{
    if (!PATH_EXISTS("freedom_injector.exe") || *rebuild_flag || is_path1_modified_after_path2("injector.cpp", "freedom_injector.exe"))
    {
        CMD("cl", "/DWIN32_LEAN_AND_MEAN", "/DNDEBUG", "/DUNICODE", "/std:c++latest", "/MT", "/O2", "/EHsc", "/nologo", "/Fe:freedom_injector.exe", "injector.cpp", "/link", "/MACHINE:x86");
    }
    async_obj_foreach_file_in_dirs(DLL_DIRS, NULL);
    if (!do_link) return;
    OBJS_FOR_DIRS(
        objs, {
            if (*debug_flag)
                { CMD("LINK", cstr_array_join(" ", objs), "/DLL", "/OUT:freedom.dll", MSVC_LINK_DEBUG_FLAGS); }
            else
                { CMD("LINK", cstr_array_join(" ", objs), "/DLL", "/OUT:freedom.dll", MSVC_LINK_RELEASE_FLAGS); }
        },
        DLL_DIRS);
}

static void build_standalone()
{
    async_obj_foreach_file_in_dirs(STANDALONE_DIRS, NULL);
    if (!do_link) return;
    OBJS_FOR_DIRS(
        objs, {
            if (*debug_flag)
            {
                CMD("LINK", cstr_array_join(" ", objs), "/OUT:freedom_standalone.exe", "vendor/GLFW/lib-vc2022/glfw3_mt.lib", "/ENTRY:mainCRTStartup", "/SUBSYSTEM:console", MSVC_LINK_DEBUG_FLAGS);
            }
            else
            {
                CMD("LINK", cstr_array_join(" ", objs), "/OUT:freedom_standalone.exe", "vendor/GLFW/lib-vc2022/glfw3_mt.lib", "/ENTRY:mainCRTStartup", "/SUBSYSTEM:windows", MSVC_LINK_RELEASE_FLAGS);
            }
        },
        STANDALONE_DIRS);
}

static void build()
{
    set_git_commit_hash();
    if (*all_flag)
    {
        build_freedom_dll();
        build_standalone();
        return;
    }
    if (*standalone_flag)
        build_standalone();
    else
        build_freedom_dll();
}

static void inject()
{
    CMD(".\\freedom_injector.exe");
}

static void launch_osu_and_inject(LPWSTR osu_exe_path)
{
    if (find_osu_exe(osu_exe_path))
    {
        if (launch_osu(osu_exe_path))
        {
            INFO("Launched osu!. Injecting in 8 seconds.");
            Sleep(8000);
            inject();
        }
        else
            WARN("Couldn't launch osu!");
    }
    else
        WARN("Couldn't find osu!.exe");
}

static void run()
{
    if (*standalone_flag)
        CMD(".\\freedom_standalone.exe");
    else
        launch_osu_and_inject(osu_exe_path);
}

static void process_args(int argc, char **argv)
{
    GO_REBUILD_URSELF(argc, argv);

    if (PATH_EXISTS(CONCAT(NOEXT(argv[0]), ".obj")))
        RM(CONCAT(NOEXT(argv[0]), ".obj"));

    standalone_flag = flag_int("standalone");
    debug_flag =      flag_int("debug");
    rebuild_flag =    flag_int("rebuild");
    run_flag =        flag_int("run");
    all_flag =        flag_int("all");
    console_flag =    flag_int("console");
    parse_flags(argc, argv);

    if (*rebuild_flag)
    {
        FOREACH_FILE_IN_DIR(file, ".", {
            if (ENDS_WITH(file, ".obj"))
                RM(file);
        });
    }
    build();
    if (*run_flag)
        run();
}

static inline void setup_vsdev_env()
{
    Find_Result result = find_visual_studio();
    if (!result.vs_exe_path)
    {
        WARN("Couldn't find Visual Studio 2017 or higher. Please run this from the MSVC x86 native tools command prompt.");
        return;
    }

    wchar_t *setup_cmd = concat4(L"\"", result.vs_exe_path, L"\" x86 && ", GetCommandLineW());
    printf("%S\n", setup_cmd);
    _wsystem(setup_cmd);

    free(setup_cmd);
    free_resources(&result);
}

int main(int argc, char **argv)
{
    getenv("VCINSTALLDIR") ? process_args(argc, argv) : setup_vsdev_env();
    return 0;
}

static void async_obj_foreach_file_in_dir(Pid *proc, size_t *proc_count, Cstr directory)
{
    FOREACH_FILE_IN_DIR(file, directory,
    {
        if (ENDS_WITH(file, ".cpp"))
        {
            Cstr src = CONCAT(directory, strdup(file));
            Cstr obj = CONCAT(NOEXT(file), ".obj");
            if (!PATH_EXISTS(obj) ||
                (*rebuild_flag || is_path1_modified_after_path2(src, obj)))
            {
                do_link = 1;
                Cstr_Array line = *debug_flag ? cstr_array_make("cl", MSVC_DEBUG_FLAGS, src, "/c", NULL) :
                                               cstr_array_make("cl", MSVC_RELEASE_FLAGS, src, "/c", NULL);
                if (define_commit_hash[0] != '\0')
                    line = cstr_array_append(line, define_commit_hash);
                if (*debug_flag)
                    line = cstr_array_append(line, "/DFR_DEBUG");
                if (*console_flag)
                    line = cstr_array_append(line, "/DFR_LOG_TO_CONSOLE");
                Cmd cmd = {.line = line};
                INFO("CMD: %s", cmd_show(cmd));
                proc[(*proc_count)++] = cmd_run_async(cmd, NULL, NULL);
            }
        }
    });
}

static void async_obj_foreach_file_in_dirs(Cstr first, ...)
{
    Pid proc[PROCESSES_CAPACITY];
    size_t proc_count = 0;
    async_obj_foreach_file_in_dir(proc, &proc_count, first);
    va_list args;
    va_start(args, first);
    for (Cstr directory = va_arg(args, Cstr);
         directory != NULL;
         directory = va_arg(args, Cstr))
    {
        async_obj_foreach_file_in_dir(proc, &proc_count, directory);
    }
    va_end(args);
    for (size_t i = 0; i < proc_count; ++i)
        pid_wait(proc[i]);
}

static inline void set_git_commit_hash()
{
    HANDLE hChildStd_OUT_Rd = NULL;
    HANDLE hChildStd_OUT_Wr = NULL;

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0);
    SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);

    TCHAR szCmdline[] = TEXT("git rev-parse --short HEAD");
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));

    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = hChildStd_OUT_Wr;
    siStartInfo.hStdInput = NULL;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    CreateProcess(NULL, szCmdline, NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo);

    WaitForSingleObject(piProcInfo.hProcess, INFINITE);

    DWORD exit_status;
    GetExitCodeProcess(piProcInfo.hProcess, &exit_status);

    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    CloseHandle(hChildStd_OUT_Wr);

    if (exit_status == 0)
    {
        DWORD dwRead;
        char git_commit_hash[16] = {0};
        ReadFile(hChildStd_OUT_Rd, git_commit_hash, 7, &dwRead, NULL);
        if (dwRead > 0)
            stbsp_snprintf(define_commit_hash, sizeof(define_commit_hash), "/DGIT_COMMIT_HASH=%s", git_commit_hash);
    }
}

static inline bool find_osu_exe(LPWSTR osu_exe_path)
{
    PWSTR roaming_path;
    if (SHGetKnownFolderPath(&FOLDERID_RoamingAppData, 0, NULL, &roaming_path) != S_OK)
        return false;
    const wchar_t *append_osu_lnk = L"\\Microsoft\\Windows\\Start Menu\\Programs\\osu!.lnk";
    size_t new_length = wcslen(roaming_path) + wcslen(append_osu_lnk) + sizeof(WCHAR);
    LPVOID new_ptr = CoTaskMemRealloc(roaming_path, new_length * sizeof(WCHAR));
    if (!new_ptr)
    {
        CoTaskMemFree(roaming_path);
        goto return_false;
    }
    roaming_path = (PWSTR)new_ptr;
    wcscat_s(roaming_path, new_length, append_osu_lnk);
    IShellLinkW *psl;
    CoInitialize(NULL);
    HRESULT hr = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLinkW, (LPVOID *)&psl);
    if (SUCCEEDED(hr))
    {
        IPersistFile *ppf;
        hr = IShellLinkW_QueryInterface(psl, &IID_IPersistFile, &ppf);
        if (SUCCEEDED(hr))
        {
            hr = IPersistFile_Load(ppf, roaming_path, STGM_READ);
            if (SUCCEEDED(hr))
            {
                WIN32_FIND_DATAW wfd;
                IShellLinkW_GetPath(psl, osu_exe_path, MAX_PATH * 2, &wfd, SLGP_UNCPRIORITY | SLGP_RAWPATH);
            }
            else
                goto return_false;
        }
        else
            goto return_false;
    }
    else
        goto return_false;

    CoUninitialize();
    CoTaskMemFree(roaming_path);
    return true;

return_false:
    CoUninitialize();
    CoTaskMemFree(roaming_path);
    return false;
}

static inline bool launch_osu(LPWSTR osu_exe_path)
{
    STARTUPINFOW siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(siStartInfo));
    siStartInfo.cb = sizeof(STARTUPINFOW);
    siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    return CreateProcessW(NULL, osu_exe_path, NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo);
}
