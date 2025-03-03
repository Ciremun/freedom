#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif // defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)

#define FLAG_IMPLEMENTATION
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define LAZER_SOURCES  "freedom/*.cpp", "freedom/lazer/*.cpp", "freedom/lazer/features/*.cpp", \
                       "vendor/imgui/*.cpp", "vendor/imgui/lazer/*.cpp"

#define LEGACY_SOURCES "freedom/*.cpp", "freedom/legacy/*.cpp", "freedom/legacy/features/*.cpp", \
                       "vendor/imgui/*.cpp", "vendor/imgui/legacy/*.cpp"

#define INCLUDE_CXXFLAGS "-Iinclude", "-Ivendor", "-Ivendor/imgui", "-Ivendor/imgui/lazer", "-Ivendor/imgui/legacy", "-Ivendor/minhook/include"

#ifdef _MSC_VER
#define COMMON_CXXFLAGS "-nologo", "-EHsc", "-D_CRT_SECURE_NO_WARNINGS", "-DWIN32_LEAN_AND_MEAN", "-DUNICODE", "-std:c++latest"
#define RELEASE_CXXFLAGS COMMON_CXXFLAGS, "-DNDEBUG", INCLUDE_CXXFLAGS, "-W3", "-O2", "-MT", "-GL"
#define DEBUG_CXXFLAGS COMMON_CXXFLAGS, INCLUDE_CXXFLAGS, "-W4", "-Od", "-Z7", "-MTd", "-FS"
static const char *default_cxx = "cl.exe";
#else
#define RELEASE_CXXFLAGS "-Wall", "-Wextra", "-pedantic", "-std=c++20", "-O3", "-DNDEBUG", INCLUDE_CXXFLAGS
#define DEBUG_CXXFLAGS "-Wall", "-Wextra", "-pedantic", "-std=c++20", "-O0", "-ggdb", INCLUDE_CXXFLAGS
static const char *default_cxx = "c++";
#endif // _MSC_VER

static bool *lazer = 0;
static bool *legacy = 0;
static bool *standalone = 0;
static bool *rebuild = 0;
static bool *debug = 0;
static bool *console = 0;
static bool *run = 0;
static bool *help = 0;
static const char *cxx = 0;
static char git_commit_hash[16] = {0};

static bool build_injector(Cmd *cmd, const char *filename)
{
#ifdef _WIN32
    filename = temp_sprintf("%s.exe", filename);
#endif // _WIN32
    if (!file_exists(filename) || *rebuild || needs_rebuild1(filename, "injector.cpp"))
    {
        nob_log(INFO, temp_sprintf("BUILD: %s", filename));
        cmd_append(cmd, cxx);
        if (!*debug)
            cmd_append(cmd, RELEASE_CXXFLAGS);
        else
            cmd_append(cmd, DEBUG_CXXFLAGS);
        cmd_append(cmd, "injector.cpp");
#ifdef _MSC_VER
        if (!*debug)
            cmd_append(cmd, "-link", temp_sprintf("-OUT:%s", filename), "-LTCG");
        else
            cmd_append(cmd, "-link", temp_sprintf("-OUT:%s", filename), "-DEBUG");
#else
        UNREACHABLE("build_injector: Not Implemented");
#endif // _MSC_VER
        return cmd_run_sync_and_reset(cmd);
    }
    return false;
}

static bool build_lazer(Cmd *cmd)
{
    build_injector(cmd, "injector-lazer");
    nob_log(INFO, "BUILD: freedom-lazer");
    cmd_append(cmd, cxx);
    if (*git_commit_hash) cmd_append(cmd, temp_sprintf("-DGIT_COMMIT_HASH=%s", git_commit_hash));
    if (*console) cmd_append(cmd, "-DFR_LOG_TO_CONSOLE");
    if (*debug) cmd_append(cmd, "-DFR_DEBUG");
    if (*lazer) cmd_append(cmd, "-DFR_LAZER");
    if (!*debug)
        cmd_append(cmd, RELEASE_CXXFLAGS);
    else
        cmd_append(cmd, DEBUG_CXXFLAGS);
    cmd_append(cmd, LAZER_SOURCES);
#ifdef _MSC_VER
    cmd_append(cmd, "-MP");
    if (!*debug)
        cmd_append(cmd, "-link", "-DLL", "-OUT:freedom-lazer.dll", "-LTCG", "-MACHINE:x64", "vendor/minhook/lib/libMinHook.x64.lib");
    else
        cmd_append(cmd, "-link", "-DLL", "-OUT:freedom-lazer.dll", "-DEBUG", "-MACHINE:x64", "vendor/minhook/lib/libMinHook.x64.lib");
#else
    UNREACHABLE("freedom-lazer: Not Implemented");
#endif // _MSC_VER
    return cmd_run_sync_and_reset(cmd);
}

static bool build_legacy(Cmd *cmd)
{
    build_injector(cmd, "injector-legacy");
    nob_log(INFO, "BUILD: freedom-legacy");
    cmd_append(cmd, cxx);
    if (*git_commit_hash) cmd_append(cmd, temp_sprintf("-DGIT_COMMIT_HASH=%s", git_commit_hash));
    if (*console) cmd_append(cmd, "-DFR_LOG_TO_CONSOLE");
    if (*debug) cmd_append(cmd, "-DFR_DEBUG");
    if (*lazer) cmd_append(cmd, "-DFR_LAZER");
    if (!*debug)
        cmd_append(cmd, RELEASE_CXXFLAGS);
    else
        cmd_append(cmd, DEBUG_CXXFLAGS);
    cmd_append(cmd, LEGACY_SOURCES);
#ifdef _MSC_VER
    // cmd_append(cmd, "-MP");
    if (!*debug)
        cmd_append(cmd, "-link", "-DLL", "-OUT:freedom-legacy.dll", "-LTCG", "-MACHINE:x86");
    else
        cmd_append(cmd, "-link", "-DLL", "-OUT:freedom-legacy.dll", "-DEBUG", "-MACHINE:x86");
#else
    UNREACHABLE("freedom-legacy: Not Implemented");
#endif // _MSC_VER
    return cmd_run_sync_and_reset(cmd);
}

static bool build()
{
    Cmd cmd = {0};
    if (*lazer && *legacy)
        return build_lazer(&cmd) && build_legacy(&cmd);
    if (*lazer)
        return build_lazer(&cmd);
    if (*legacy)
        return build_legacy(&cmd);
    UNREACHABLE("build");
}

static bool run_executable(const char *executable)
{
    Cmd cmd = {0};
#ifdef _WIN32
    cmd_append(&cmd, temp_sprintf(".\\%s.exe", executable));
#else
    cmd_append(&cmd, temp_sprintf("./%s", executable));
#endif // _WIN32
    return cmd_run_sync(cmd);
}

static void usage(FILE *stream)
{
#ifdef _WIN32
    fprintf(stream, "Usage: .\\nob.exe [OPTIONS]\n");
#else
    fprintf(stream, "Usage: ./nob [OPTIONS]\n");
#endif // _WIN32
    fprintf(stream, "OPTIONS:\n    (no options)\n        build all\n");
    flag_print_options(stream);
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    lazer = flag_bool("lazer", false, "build freedom-lazer");
    legacy = flag_bool("legacy", false, "build freedom-legacy");
    standalone = flag_bool("standalone", false, "build freedom-standalone (ui demo)");
    rebuild = flag_bool("rebuild", false, "clean build / update headers");
    debug = flag_bool("debug", false, "symbols, disable optimizations");
    console = flag_bool("console", false, "use console log at runtime");
    run = flag_bool("run", false, "run osu and inject / run freedom-standalone");
    help = flag_bool("help", false, "print help and exit");

    if (!flag_parse(argc, argv)) {
        usage(stderr);
        flag_print_error(stderr);
        return 1;
    }

    if (*help) {
        usage(stdout);
        return 0;
    }

    if (!*lazer && !*legacy) {
#if defined(_WIN64)
        *lazer = true;
#elif defined(_WIN32)
        *legacy = true;
#else
        *lazer = true;
        *legacy = true;
#endif // defined(_WIN64)
    }

    if (!(cxx = getenv("CXX")))
        cxx = default_cxx;

    Nob_String_Builder file = {0};
    if (read_entire_file(".git/refs/heads/master", &file)) {
        memcpy(git_commit_hash, file.items, 7);
    }

    build();

    if (*run) {
        if (*lazer)
            run_executable("injector-lazer");
        if (*legacy)
            run_executable("injector-legacy");
        if (*standalone)
            run_executable("freedom-standalone");
    }

    return 0;
}
