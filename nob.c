#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif // defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)

#define FLAG_IMPLEMENTATION
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#ifdef _MSC_VER
#define RELEASE_CXXFLAGS "-nologo", "-W4", "-std:c++11"
static const char *default_cxx = "cl.exe";
#else
#define RELEASE_CXXFLAGS "-Wall", "-Wextra", "-pedantic", "-std=c++11", "-O0", "-ggdb"
static const char *default_cxx = "c++";
#endif // _MSC_VER

static const char *cxx = 0;

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

    bool *lazer = flag_bool("lazer", false, "build freedom-lazer");
    bool *legacy = flag_bool("legacy", false, "build freedom-legacy");
    bool *standalone = flag_bool("standalone", false, "build standalone ui demo");
    bool *rebuild = flag_bool("rebuild", false, "clean build / update headers");
    bool *debug = flag_bool("debug", false, "symbols, disable optimizations");
    bool *console = flag_bool("console", false, "use console log at runtime");
    bool *run = flag_bool("run", false, "run osu and inject / run standalone ui demo");
    bool *help = flag_bool("help", false, "print help and exit");

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
        *lazer = true;
        *legacy = true;
    }

    if (!(cxx = getenv("CXX")))
        cxx = default_cxx;
    nob_log(INFO, "CXX: %s", cxx);

    nob_log(INFO, "lazer: %d", *lazer);
    nob_log(INFO, "legacy: %d", *legacy);

    Nob_String_Builder file = {0};
    if (read_entire_file(".git/refs/heads/master", &file)) {
        nob_log(INFO, "git commit hash: %s", file.items);
    }

    return 0;
}
