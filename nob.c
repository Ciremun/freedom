#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif // defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)

#define FLAG_IMPLEMENTATION
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define LAZER_SOURCES  "freedom/*.cpp", "freedom/lazer/*.cpp", "freedom/lazer/features/*.cpp", "freedom/lazer/events/*.cpp", \
                       "vendor/imgui/*.cpp", "vendor/imgui/lazer/*.cpp"

#define LEGACY_SOURCES "freedom/*.cpp", "freedom/legacy/*.cpp", "freedom/legacy/features/*.cpp", \
                       "vendor/imgui/*.cpp", "vendor/imgui/legacy/*.cpp"

#define DEMO_SOURCES "freedom/*.cpp", "freedom/lazer/ui.cpp", "freedom/demo/*.cpp", \
                     "vendor/imgui/*.cpp", "vendor/imgui/demo/*.cpp", "vendor/imgui/legacy/*.cpp", \
                     "vendor/imgui/lazer/*.cpp"

#define INCLUDE_CXXFLAGS "-Iinclude", "-Ivendor", "-Ivendor/imgui"

#ifdef _MSC_VER
#define COMMON_CXXFLAGS "-D_CRT_SECURE_NO_WARNINGS", "-DWIN32_LEAN_AND_MEAN", "-DUNICODE", "-nologo", "-EHsc", "-std:c++latest"
#define RELEASE_CXXFLAGS COMMON_CXXFLAGS, "-DNDEBUG", "-W3", "-O2", "-MT", "-GL", INCLUDE_CXXFLAGS
#define DEBUG_CXXFLAGS COMMON_CXXFLAGS, "-W4", "-Od", "-Z7", "-MTd", "-FS", INCLUDE_CXXFLAGS
static const char *default_cxx = "cl.exe";
#else
#define RELEASE_CXXFLAGS "-Wall", "-Wextra", "-pedantic", "-std=c++20", "-O3", "-DNDEBUG", INCLUDE_CXXFLAGS
#define DEBUG_CXXFLAGS "-Wall", "-Wextra", "-pedantic", "-std=c++20", "-O0", "-ggdb", INCLUDE_CXXFLAGS
static const char *default_cxx = "c++";
#endif // _MSC_VER

static bool *lazer = 0;
static bool *legacy = 0;
static bool *demo = 0;
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
            cmd_append(cmd, "-link", temp_sprintf("-OUT:%s", filename));
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
    cmd_append(cmd, "-DFR_LAZER");
    if (!*debug)
        cmd_append(cmd, RELEASE_CXXFLAGS);
    else
        cmd_append(cmd, DEBUG_CXXFLAGS);
    cmd_append(cmd, "-Ivendor/imgui/lazer");
    cmd_append(cmd, LAZER_SOURCES);
#ifdef _MSC_VER
    cmd_append(cmd, "-MP");
    if (!*debug)
        cmd_append(cmd, "-link", "-DLL", "-OUT:freedom-lazer.dll", "-LTCG", "-MACHINE:x64", "vendor/minhook/minhook.x64.mt.lib");
    else
        cmd_append(cmd, "-link", "-DLL", "-OUT:freedom-lazer.dll", "-LTCG", "-MACHINE:x64", "vendor/minhook/minhook.x64.mtd.lib");
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
    if (!*debug)
        cmd_append(cmd, RELEASE_CXXFLAGS);
    else
        cmd_append(cmd, DEBUG_CXXFLAGS);
    cmd_append(cmd, "-Ivendor/imgui/legacy");
    Cmd no_sources;
    memcpy(&no_sources, cmd, sizeof(Cmd));
    cmd_append(cmd, LEGACY_SOURCES);
#ifdef _MSC_VER
    // NOTE(Ciremun): Build compatible sources in parallel
    File_Paths children = {0};
    if (!read_entire_dir(".", &children))
        return false;
    for (size_t i = 0; i < children.count; ++i)
        if (sv_end_with(sv_from_cstr(children.items[i]), ".obj"))
            delete_file(children.items[i]);
    cmd_append(cmd, "-MP", "-c");
    if (!cmd_run_sync_and_reset(cmd))
        return false;
    // NOTE(Ciremun): Build incompatible CLR stuff
    cmd_append(&no_sources, "-c", "freedom/legacy/clr/*.cpp");
    if (!cmd_run_sync_and_reset(&no_sources))
        return false;
    // NOTE(Ciremun): Link
    if (!*debug)
        cmd_append(cmd, "LINK", "*.obj", "-DLL", "-OUT:freedom-legacy.dll", "-LTCG", "-MACHINE:x86");
    else
        cmd_append(cmd, "LINK", "*.obj", "-DLL", "-OUT:freedom-legacy.dll", "-MACHINE:x86");
    return cmd_run_sync_and_reset(cmd);
#else
    UNREACHABLE("freedom-legacy: Not Implemented");
#endif // _MSC_VER
    return cmd_run_sync_and_reset(cmd);
}

static bool build_ui_demo(Cmd *cmd)
{
    nob_log(INFO, "BUILD: freedom-ui-demo");
    cmd_append(cmd, cxx);
    if (*git_commit_hash) cmd_append(cmd, temp_sprintf("-DGIT_COMMIT_HASH=%s", git_commit_hash));
    if (*console) cmd_append(cmd, "-DFR_LOG_TO_CONSOLE");
    if (*debug) cmd_append(cmd, "-DFR_DEBUG");
    cmd_append(cmd, "-DFR_LAZER");
    if (!*debug)
        cmd_append(cmd, RELEASE_CXXFLAGS);
    else
        cmd_append(cmd, DEBUG_CXXFLAGS);
    cmd_append(cmd, "-Ivendor/imgui/demo", "-Ivendor/imgui/legacy", "-Ivendor/imgui/lazer");
    cmd_append(cmd, DEMO_SOURCES);
#ifdef _MSC_VER
    cmd_append(cmd, "-MP");
    if (!*debug)
        cmd_append(cmd, "-link", "-OUT:freedom-ui-demo.exe", "-LTCG", "-MACHINE:x64", "vendor/GLFW/glfw3.x64.mt.lib");
    else
        cmd_append(cmd, "-link", "-OUT:freedom-ui-demo.exe", "-MACHINE:x64", "vendor/GLFW/glfw3.x64.mt.lib");
#else
    UNREACHABLE("freedom-ui-demo: Not Implemented");
#endif // _MSC_VER
    return cmd_run_sync_and_reset(cmd);
}

static bool build()
{
    Cmd cmd = {0};
#ifndef _MSC_VER
    if (*lazer && *legacy)
        return build_lazer(&cmd) && build_legacy(&cmd);
#endif // _MSC_VER
    if (*lazer)
        return build_lazer(&cmd);
    if (*legacy)
        return build_legacy(&cmd);
    if (*demo)
        return build_ui_demo(&cmd);
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
    demo = flag_bool("demo", false, "build freedom-ui-demo");
    rebuild = flag_bool("rebuild", false, "clean build");
    debug = flag_bool("debug", false, "symbols, disable optimizations");
    console = flag_bool("console", false, "use console log at runtime");
    run = flag_bool("run", false, "inject after build / run freedom-ui-demo");
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

    if (!*lazer && !*legacy && !*demo) {
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

    String_Builder file = {0};
    if (read_entire_file(".git/refs/heads/master", &file) && file.count >= 7) {
        memcpy(git_commit_hash, file.items, 7);
    }

    bool ok = build();

    if (*run && ok) {
        if (*lazer)
            run_executable("injector-lazer");
        if (*legacy)
            run_executable("injector-legacy");
        if (*demo)
            run_executable("freedom-ui-demo");
    }

    return 0;
}
