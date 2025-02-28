#define FLAG_IMPLEMENTATION
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#ifdef _MSC_VER
#define RELEASE_CXXFLAGS "/nologo", "/W3", "/std:c++11"
static const char *default_cxx = "cl.exe";
#else
#define RELEASE_CXXFLAGS "-Wall", "-Wextra", "-pedantic", "-std=c++11", "-O0", "-ggdb"
static const char *default_cxx = "c++";
#endif // _MSC_VER

static const char *cxx = 0;

static bool run_executable(const char *executable)
{
    Cmd cmd = {0};
    String_Builder sb = {0};
#ifdef _WIN32
    sb_append_cstr(&sb, ".\\");
    sb_append_cstr(&sb, executable);
    sb_append_cstr(&sb, ".exe");
#else
    sb_append_cstr(&sb, "./");
    sb_append_cstr(&sb, executable);
#endif // _WIN32
    sb_append_null(&sb);
    cmd_append(&cmd, sb.items);
    return cmd_run_sync(cmd);
}

static void usage(FILE *stream)
{
#ifdef _WIN32
    fprintf(stream, "Usage: nob.exe [OPTIONS]\n");
#else
    fprintf(stream, "Usage: ./nob [OPTIONS]\n");
#endif // _WIN32
    fprintf(stream, "OPTIONS:\n");
    flag_print_options(stream);
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    bool *lazer = flag_bool("lazer", false, "build freedom-lazer");
    bool *legacy = flag_bool("legacy", false, "build freedom-legacy");
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

    if (!(cxx = getenv("cxx")))
        cxx = default_cxx;
    nob_log(INFO, "CXX: %s", cxx);

    fprintf(stdout, "lazer: %d\n", *lazer);
    fprintf(stdout, "legacy: %d\n", *legacy);

    return 0;
}
