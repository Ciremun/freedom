#ifndef NOBUILD_H_
#define NOBUILD_H_

#ifndef _WIN32
#    define _POSIX_C_SOURCE 200809L
#    include <sys/types.h>
#    include <sys/wait.h>
#    include <sys/stat.h>
#    include <unistd.h>
#    include <dirent.h>
#    include <fcntl.h>
#    define PATH_SEP "/"
typedef pid_t Pid;
typedef int Fd;
#else
#    define WIN32_MEAN_AND_LEAN
#    ifndef COBJMACROS
#        define COBJMACROS
#    endif // COBJMACROS
#    include <windows.h>
#    include <shlobj.h>
#    pragma comment(lib, "Shell32.lib")
#    include <process.h>
#    define PATH_SEP "\\"
typedef HANDLE Pid;
typedef HANDLE Fd;
// minirent.h HEADER BEGIN ////////////////////////////////////////
// Copyright 2021 Alexey Kutepov <reximkut@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// ============================================================
//
// minirent — 0.0.1 — A subset of dirent interface for Windows.
//
// https://github.com/tsoding/minirent
//
// ============================================================
//
// ChangeLog (https://semver.org/ is implied)
//
//    0.0.1 First Official Release

#ifndef MINIRENT_H_
#define MINIRENT_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct dirent {
    char d_name[MAX_PATH+1];
};

typedef struct DIR DIR;

DIR *opendir(const char *dirpath);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);

#endif  // MINIRENT_H_
// minirent.h HEADER END ////////////////////////////////////////

// TODO(#28): use GetLastErrorAsString everywhere on Windows error reporting
LPSTR GetLastErrorAsString(void);

#endif  // _WIN32

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#define FOREACH_ARRAY(type, elem, array, body)  \
    for (size_t elem_##index = 0;                           \
         elem_##index < array.count;                        \
         ++elem_##index)                                    \
    {                                                       \
        type *elem = &array.elems[elem_##index];            \
        body;                                               \
    }

typedef const char * Cstr;

int cstr_ends_with(Cstr cstr, Cstr postfix);
#define ENDS_WITH(cstr, postfix) cstr_ends_with(cstr, postfix)

Cstr cstr_no_ext(Cstr path);
#define NOEXT(path) cstr_no_ext(path)

typedef struct {
    Cstr *elems;
    size_t count;
} Cstr_Array;

Cstr_Array cstr_array_make(Cstr first, ...);
Cstr_Array cstr_array_append(Cstr_Array cstrs, Cstr cstr);
Cstr cstr_array_join(Cstr sep, Cstr_Array cstrs);

#define JOIN(sep, ...) cstr_array_join(sep, cstr_array_make(__VA_ARGS__, NULL))
#define CONCAT(...) JOIN("", __VA_ARGS__)
#define PATH(...) JOIN(PATH_SEP, __VA_ARGS__)

typedef struct {
    Fd read;
    Fd write;
} Pipe;

Pipe pipe_make(void);

typedef struct {
    Cstr_Array line;
} Cmd;

Fd fd_open_for_read(Cstr path);
Fd fd_open_for_write(Cstr path);
void fd_close(Fd fd);
void pid_wait(Pid pid);
Cstr cmd_show(Cmd cmd);
Pid cmd_run_async(Cmd cmd, Fd *fdin, Fd *fdout);
void cmd_run_sync(Cmd cmd);

typedef struct {
    Cmd *elems;
    size_t count;
} Cmd_Array;

// TODO(#1): no way to disable echo in nobuild scripts
// TODO(#2): no way to ignore fails
#define CMD(...)                                        \
    do {                                                \
        Cmd cmd = {                                     \
            .line = cstr_array_make(__VA_ARGS__, NULL)  \
        };                                              \
        INFO("CMD: %s", cmd_show(cmd));                 \
        cmd_run_sync(cmd);                              \
    } while (0)

typedef enum {
    CHAIN_TOKEN_END = 0,
    CHAIN_TOKEN_IN,
    CHAIN_TOKEN_OUT,
    CHAIN_TOKEN_CMD
} Chain_Token_Type;

// A single token for the CHAIN(...) DSL syntax
typedef struct {
    Chain_Token_Type type;
    Cstr_Array args;
} Chain_Token;

#define CHAIN_CMD(...) \
    (Chain_Token) { \
        .type = CHAIN_TOKEN_CMD, \
        .args = cstr_array_make(__VA_ARGS__, NULL) \
    }

// TODO(#20): pipes do not allow redirecting stderr
typedef struct {
    Cstr input_filepath;
    Cmd_Array cmds;
    Cstr output_filepath;
} Chain;

Chain chain_build_from_tokens(Chain_Token first, ...);
void chain_run_sync(Chain chain);
void chain_echo(Chain chain);

// TODO(#15): PIPE does not report where exactly a syntactic error has happened
#define CHAIN(...)                                                      \
    do {                                                                \
        Chain chain = chain_build_from_tokens(__VA_ARGS__, (Chain_Token) {0}); \
        chain_echo(chain);                                              \
        chain_run_sync(chain);                                          \
    } while(0)

#ifndef REBUILD_URSELF
#  if _WIN32
#    if defined(__GNUC__)
#       define REBUILD_URSELF(binary_path, source_path) CMD("gcc", "-o", binary_path, source_path)
#    elif defined(__clang__)
#       define REBUILD_URSELF(binary_path, source_path) CMD("clang", "-o", binary_path, source_path)
#    elif defined(_MSC_VER)
#       define REBUILD_URSELF(binary_path, source_path) CMD("cl.exe", source_path)
#    endif
#  else
#    define REBUILD_URSELF(binary_path, source_path) CMD("cc", "-o", binary_path, source_path)
#  endif
#endif

// Go Rebuild Urself™ Technology
//
//   How to use it:
//     int main(int argc, char** argv) {
//         GO_REBUILD_URSELF(argc, argv);
//         // actual work
//         return 0;
//     }
//
//   After your added this macro every time you run ./nobuild it will detect
//   that you modified its original source code and will try to rebuild itself
//   before doing any actual work. So you only need to bootstrap your build system
//   once.
//
//   The modification is detected by comparing the last modified times of the executable
//   and its source code. The same way the make utility usually does it.
//
//   The rebuilding is done by using the REBUILD_URSELF macro which you can redefine
//   if you need a special way of bootstraping your build system. (which I personally
//   do not recommend since the whole idea of nobuild is to keep the process of bootstrapping
//   as simple as possible and doing all of the actual work inside of the nobuild)
//
#define GO_REBUILD_URSELF(argc, argv)                                  \
    do {                                                               \
        const char *source_path = __FILE__;                            \
        assert(argc >= 1);                                             \
        const char *binary_path = argv[0];                             \
                                                                       \
        if (is_path1_modified_after_path2(source_path, binary_path)) { \
            RENAME(binary_path, CONCAT(binary_path, ".old"));          \
            REBUILD_URSELF(binary_path, source_path);                  \
            Cmd cmd = {                                                \
                .line = {                                              \
                    .elems = (Cstr*) argv,                             \
                    .count = argc,                                     \
                },                                                     \
            };                                                         \
            INFO("CMD: %s", cmd_show(cmd));                            \
            cmd_run_sync(cmd);                                         \
            exit(0);                                                   \
        }                                                              \
    } while(0)
// The implementation idea is stolen from https://github.com/zhiayang/nabs

void rebuild_urself(const char *binary_path, const char *source_path);

int path_is_dir(Cstr path);
#define IS_DIR(path) path_is_dir(path)

int path_exists(Cstr path);
#define PATH_EXISTS(path) path_exists(path)

void path_mkdirs(Cstr_Array path);
#define MKDIRS(...)                                             \
    do {                                                        \
        Cstr_Array path = cstr_array_make(__VA_ARGS__, NULL);   \
        INFO("MKDIRS: %s", cstr_array_join(PATH_SEP, path));    \
        path_mkdirs(path);                                      \
    } while (0)

void path_rename(Cstr old_path, Cstr new_path);
#define RENAME(old_path, new_path)                    \
    do {                                              \
        INFO("RENAME: %s -> %s", old_path, new_path); \
        path_rename(old_path, new_path);              \
    } while (0)

void path_rm(Cstr path);
#define RM(path)                                \
    do {                                        \
        INFO("RM: %s", path);                   \
        path_rm(path);                          \
    } while(0)

#define FOREACH_FILE_IN_DIR(file, dirpath, body)        \
    do {                                                \
        struct dirent *dp = NULL;                       \
        DIR *dir = opendir(dirpath);                    \
        if (dir == NULL) {                              \
            PANIC("could not open directory %s: %s",    \
                  dirpath, strerror(errno));            \
        }                                               \
        errno = 0;                                      \
        while ((dp = readdir(dir))) {                   \
            const char *file = dp->d_name;              \
            body;                                       \
        }                                               \
                                                        \
        if (errno > 0) {                                \
            PANIC("could not read directory %s: %s",    \
                  dirpath, strerror(errno));            \
        }                                               \
                                                        \
        closedir(dir);                                  \
    } while(0)

#if defined(__GNUC__) || defined(__clang__)
// https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Function-Attributes.html
#define NOBUILD_PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK) __attribute__ ((format (printf, STRING_INDEX, FIRST_TO_CHECK)))
#else
#define NOBUILD_PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK)
#endif

void VLOG(FILE *stream, Cstr tag, Cstr fmt, va_list args);
void INFO(Cstr fmt, ...) NOBUILD_PRINTF_FORMAT(1, 2);
void WARN(Cstr fmt, ...) NOBUILD_PRINTF_FORMAT(1, 2);
void ERRO(Cstr fmt, ...) NOBUILD_PRINTF_FORMAT(1, 2);
void PANIC(Cstr fmt, ...) NOBUILD_PRINTF_FORMAT(1, 2);

char *shift_args(int *argc, char ***argv);

#endif  // NOBUILD_H_

////////////////////////////////////////////////////////////////////////////////

#ifdef NOBUILD_IMPLEMENTATION

#ifdef _WIN32
LPSTR GetLastErrorAsString(void)
{
    // https://stackoverflow.com/questions/1387064/how-to-get-the-error-message-from-the-error-code-returned-by-getlasterror

    DWORD errorMessageId = GetLastError();
    assert(errorMessageId != 0);

    LPSTR messageBuffer = NULL;

    DWORD size =
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, // DWORD   dwFlags,
            NULL, // LPCVOID lpSource,
            errorMessageId, // DWORD   dwMessageId,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // DWORD   dwLanguageId,
            (LPSTR) &messageBuffer, // LPTSTR  lpBuffer,
            0, // DWORD   nSize,
            NULL // va_list *Arguments
        );

    return messageBuffer;
}

// minirent.h IMPLEMENTATION BEGIN ////////////////////////////////////////
struct DIR {
    HANDLE hFind;
    WIN32_FIND_DATA data;
    struct dirent *dirent;
};

DIR *opendir(const char *dirpath)
{
    assert(dirpath);

    char buffer[MAX_PATH];
    snprintf(buffer, MAX_PATH, "%s\\*", dirpath);

    DIR *dir = (DIR*)calloc(1, sizeof(DIR));

    dir->hFind = FindFirstFile(buffer, &dir->data);
    if (dir->hFind == INVALID_HANDLE_VALUE) {
        errno = ENOSYS;
        goto fail;
    }

    return dir;

fail:
    if (dir) {
        free(dir);
    }

    return NULL;
}

struct dirent *readdir(DIR *dirp)
{
    assert(dirp);

    if (dirp->dirent == NULL) {
        dirp->dirent = (struct dirent*)calloc(1, sizeof(struct dirent));
    } else {
        if(!FindNextFile(dirp->hFind, &dirp->data)) {
            if (GetLastError() != ERROR_NO_MORE_FILES) {
                errno = ENOSYS;
            }

            return NULL;
        }
    }

    memset(dirp->dirent->d_name, 0, sizeof(dirp->dirent->d_name));

    strncpy(
        dirp->dirent->d_name,
        dirp->data.cFileName,
        sizeof(dirp->dirent->d_name) - 1);

    return dirp->dirent;
}

int closedir(DIR *dirp)
{
    assert(dirp);

    if(!FindClose(dirp->hFind)) {
        errno = ENOSYS;
        return -1;
    }

    if (dirp->dirent) {
        free(dirp->dirent);
    }
    free(dirp);

    return 0;
}
// minirent.h IMPLEMENTATION END ////////////////////////////////////////
#endif // _WIN32

Cstr_Array cstr_array_append(Cstr_Array cstrs, Cstr cstr)
{
    Cstr_Array result = {
        .count = cstrs.count + 1
    };
    result.elems = malloc(sizeof(result.elems[0]) * result.count);
    memcpy(result.elems, cstrs.elems, cstrs.count * sizeof(result.elems[0]));
    result.elems[cstrs.count] = cstr;
    return result;
}

int cstr_ends_with(Cstr cstr, Cstr postfix)
{
    const size_t cstr_len = strlen(cstr);
    const size_t postfix_len = strlen(postfix);
    return postfix_len <= cstr_len
           && strcmp(cstr + cstr_len - postfix_len, postfix) == 0;
}

Cstr cstr_no_ext(Cstr path)
{
    size_t n = strlen(path);
    while (n > 0 && path[n - 1] != '.') {
        n -= 1;
    }

    if (n > 0) {
        char *result = malloc(n);
        memcpy(result, path, n);
        result[n - 1] = '\0';

        return result;
    } else {
        return path;
    }
}

Cstr_Array cstr_array_make(Cstr first, ...)
{
    Cstr_Array result = {0};

    if (first == NULL) {
        return result;
    }

    result.count += 1;

    va_list args;
    va_start(args, first);
    for (Cstr next = va_arg(args, Cstr);
            next != NULL;
            next = va_arg(args, Cstr)) {
        result.count += 1;
    }
    va_end(args);

    result.elems = malloc(sizeof(result.elems[0]) * result.count);
    if (result.elems == NULL) {
        PANIC("could not allocate memory: %s", strerror(errno));
    }
    result.count = 0;

    result.elems[result.count++] = first;

    va_start(args, first);
    for (Cstr next = va_arg(args, Cstr);
            next != NULL;
            next = va_arg(args, Cstr)) {
        result.elems[result.count++] = next;
    }
    va_end(args);

    return result;
}

Cstr cstr_array_join(Cstr sep, Cstr_Array cstrs)
{
    if (cstrs.count == 0) {
        return "";
    }

    const size_t sep_len = strlen(sep);
    size_t len = 0;
    for (size_t i = 0; i < cstrs.count; ++i) {
        len += strlen(cstrs.elems[i]);
    }

    const size_t result_len = (cstrs.count - 1) * sep_len + len + 1;
    char *result = malloc(sizeof(char) * result_len);
    if (result == NULL) {
        PANIC("could not allocate memory: %s", strerror(errno));
    }

    len = 0;
    for (size_t i = 0; i < cstrs.count; ++i) {
        if (i > 0) {
            memcpy(result + len, sep, sep_len);
            len += sep_len;
        }

        size_t elem_len = strlen(cstrs.elems[i]);
        memcpy(result + len, cstrs.elems[i], elem_len);
        len += elem_len;
    }
    result[len] = '\0';

    return result;
}

Pipe pipe_make(void)
{
    Pipe pip = {0};

#ifdef _WIN32
    // https://docs.microsoft.com/en-us/windows/win32/ProcThread/creating-a-child-process-with-redirected-input-and-output

    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    if (!CreatePipe(&pip.read, &pip.write, &saAttr, 0)) {
        PANIC("Could not create pipe: %s", GetLastErrorAsString());
    }
#else
    Fd pipefd[2];
    if (pipe(pipefd) < 0) {
        PANIC("Could not create pipe: %s", strerror(errno));
    }

    pip.read = pipefd[0];
    pip.write = pipefd[1];
#endif // _WIN32

    return pip;
}

Fd fd_open_for_read(Cstr path)
{
#ifndef _WIN32
    Fd result = open(path, O_RDONLY);
    if (result < 0) {
        PANIC("Could not open file %s: %s", path, strerror(errno));
    }
    return result;
#else
    // https://docs.microsoft.com/en-us/windows/win32/fileio/opening-a-file-for-reading-or-writing
    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    Fd result = CreateFile(
                    path,
                    GENERIC_READ,
                    0,
                    &saAttr,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_READONLY,
                    NULL);

    if (result == INVALID_HANDLE_VALUE) {
        PANIC("Could not open file %s", path);
    }

    return result;
#endif // _WIN32
}

Fd fd_open_for_write(Cstr path)
{
#ifndef _WIN32
    Fd result = open(path,
                     O_WRONLY | O_CREAT | O_TRUNC,
                     S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (result < 0) {
        PANIC("could not open file %s: %s", path, strerror(errno));
    }
    return result;
#else
    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    Fd result = CreateFile(
                    path,                  // name of the write
                    GENERIC_WRITE,         // open for writing
                    0,                     // do not share
                    &saAttr,               // default security
                    CREATE_NEW,            // create new file only
                    FILE_ATTRIBUTE_NORMAL, // normal file
                    NULL                   // no attr. template
                );

    if (result == INVALID_HANDLE_VALUE) {
        PANIC("Could not open file %s: %s", path, GetLastErrorAsString());
    }

    return result;
#endif // _WIN32
}

void fd_close(Fd fd)
{
#ifdef _WIN32
    CloseHandle(fd);
#else
    close(fd);
#endif // _WIN32
}

void pid_wait(Pid pid)
{
#ifdef _WIN32
    DWORD result = WaitForSingleObject(
                       pid,     // HANDLE hHandle,
                       INFINITE // DWORD  dwMilliseconds
                   );

    if (result == WAIT_FAILED) {
        PANIC("could not wait on child process: %s", GetLastErrorAsString());
    }

    DWORD exit_status;
    if (GetExitCodeProcess(pid, &exit_status) == 0) {
        PANIC("could not get process exit code: %lu", GetLastError());
    }

    if (exit_status != 0) {
        PANIC("command exited with exit code %lu", exit_status);
    }

    CloseHandle(pid);
#else
    for (;;) {
        int wstatus = 0;
        if (waitpid(pid, &wstatus, 0) < 0) {
            PANIC("could not wait on command (pid %d): %s", pid, strerror(errno));
        }

        if (WIFEXITED(wstatus)) {
            int exit_status = WEXITSTATUS(wstatus);
            if (exit_status != 0) {
                PANIC("command exited with exit code %d", exit_status);
            }

            break;
        }

        if (WIFSIGNALED(wstatus)) {
            PANIC("command process was terminated by %s", strsignal(WTERMSIG(wstatus)));
        }
    }

#endif // _WIN32
}

Cstr cmd_show(Cmd cmd)
{
    // TODO(#31): cmd_show does not render the command line properly
    // - No string literals when arguments contains space
    // - No escaping of special characters
    // - Etc.
    return cstr_array_join(" ", cmd.line);
}

Pid cmd_run_async(Cmd cmd, Fd *fdin, Fd *fdout)
{
#ifdef _WIN32
    // https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output

    STARTUPINFO siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(siStartInfo));
    siStartInfo.cb = sizeof(STARTUPINFO);
    // NOTE: theoretically setting NULL to std handles should not be a problem
    // https://docs.microsoft.com/en-us/windows/console/getstdhandle?redirectedfrom=MSDN#attachdetach-behavior
    siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    // TODO(#32): check for errors in GetStdHandle
    siStartInfo.hStdOutput = fdout ? *fdout : GetStdHandle(STD_OUTPUT_HANDLE);
    siStartInfo.hStdInput = fdin ? *fdin : GetStdHandle(STD_INPUT_HANDLE);
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    BOOL bSuccess =
        CreateProcess(
            NULL,
            // TODO(#33): cmd_run_async on Windows does not render command line properly
            // It may require wrapping some arguments with double-quotes if they contains spaces, etc.
            (char *)cstr_array_join(" ", cmd.line),
            NULL,
            NULL,
            TRUE,
            0,
            NULL,
            NULL,
            &siStartInfo,
            &piProcInfo
        );

    if (!bSuccess) {
        PANIC("Could not create child process %s: %s\n",
              cmd_show(cmd), GetLastErrorAsString());
    }

    CloseHandle(piProcInfo.hThread);

    return piProcInfo.hProcess;
#else
    pid_t cpid = fork();
    if (cpid < 0) {
        PANIC("Could not fork child process: %s: %s",
              cmd_show(cmd), strerror(errno));
    }

    if (cpid == 0) {
        Cstr_Array args = cstr_array_append(cmd.line, NULL);

        if (fdin) {
            if (dup2(*fdin, STDIN_FILENO) < 0) {
                PANIC("Could not setup stdin for child process: %s", strerror(errno));
            }
        }

        if (fdout) {
            if (dup2(*fdout, STDOUT_FILENO) < 0) {
                PANIC("Could not setup stdout for child process: %s", strerror(errno));
            }
        }

        if (execvp(args.elems[0], (char * const*) args.elems) < 0) {
            PANIC("Could not exec child process: %s: %s",
                  cmd_show(cmd), strerror(errno));
        }
    }

    return cpid;
#endif // _WIN32
}

void cmd_run_sync(Cmd cmd)
{
    pid_wait(cmd_run_async(cmd, NULL, NULL));
}

static void chain_set_input_output_files_or_count_cmds(Chain *chain, Chain_Token token)
{
    switch (token.type) {
    case CHAIN_TOKEN_CMD: {
        chain->cmds.count += 1;
    }
    break;

    case CHAIN_TOKEN_IN: {
        if (chain->input_filepath) {
            PANIC("Input file path was already set");
        }

        chain->input_filepath = token.args.elems[0];
    }
    break;

    case CHAIN_TOKEN_OUT: {
        if (chain->output_filepath) {
            PANIC("Output file path was already set");
        }

        chain->output_filepath = token.args.elems[0];
    }
    break;

    case CHAIN_TOKEN_END:
    default: {
        assert(0 && "unreachable");
        exit(1);
    }
    }
}

static void chain_push_cmd(Chain *chain, Chain_Token token)
{
    if (token.type == CHAIN_TOKEN_CMD) {
        chain->cmds.elems[chain->cmds.count++] = (Cmd) {
            .line = token.args
        };
    }
}

Chain chain_build_from_tokens(Chain_Token first, ...)
{
    Chain result = {0};

    chain_set_input_output_files_or_count_cmds(&result, first);
    va_list args;
    va_start(args, first);
    Chain_Token next = va_arg(args, Chain_Token);
    while (next.type != CHAIN_TOKEN_END) {
        chain_set_input_output_files_or_count_cmds(&result, next);
        next = va_arg(args, Chain_Token);
    }
    va_end(args);

    result.cmds.elems = malloc(sizeof(result.cmds.elems[0]) * result.cmds.count);
    if (result.cmds.elems == NULL) {
        PANIC("could not allocate memory: %s", strerror(errno));
    }
    result.cmds.count = 0;

    chain_push_cmd(&result, first);

    va_start(args, first);
    next = va_arg(args, Chain_Token);
    while (next.type != CHAIN_TOKEN_END) {
        chain_push_cmd(&result, next);
        next = va_arg(args, Chain_Token);
    }
    va_end(args);

    return result;
}

void chain_run_sync(Chain chain)
{
    if (chain.cmds.count == 0) {
        return;
    }

    Pid *cpids = malloc(sizeof(Pid) * chain.cmds.count);

    Pipe pip = {0};
    Fd fdin = 0;
    Fd *fdprev = NULL;

    if (chain.input_filepath) {
        fdin = fd_open_for_read(chain.input_filepath);
        if (fdin < 0) {
            PANIC("could not open file %s: %s", chain.input_filepath, strerror(errno));
        }
        fdprev = &fdin;
    }

    for (size_t i = 0; i < chain.cmds.count - 1; ++i) {
        pip = pipe_make();

        cpids[i] = cmd_run_async(
                       chain.cmds.elems[i],
                       fdprev,
                       &pip.write);

        if (fdprev) fd_close(*fdprev);
        fd_close(pip.write);
        fdprev = &fdin;
        fdin = pip.read;
    }

    {
        Fd fdout = 0;
        Fd *fdnext = NULL;

        if (chain.output_filepath) {
            fdout = fd_open_for_write(chain.output_filepath);
            if (fdout < 0) {
                PANIC("could not open file %s: %s",
                      chain.output_filepath,
                      strerror(errno));
            }
            fdnext = &fdout;
        }

        const size_t last = chain.cmds.count - 1;
        cpids[last] =
            cmd_run_async(
                chain.cmds.elems[last],
                fdprev,
                fdnext);

        if (fdprev) fd_close(*fdprev);
        if (fdnext) fd_close(*fdnext);
    }

    for (size_t i = 0; i < chain.cmds.count; ++i) {
        pid_wait(cpids[i]);
    }
}

void chain_echo(Chain chain)
{
    printf("[INFO] CHAIN:");
    if (chain.input_filepath) {
        printf(" %s", chain.input_filepath);
    }

    FOREACH_ARRAY(Cmd, cmd, chain.cmds, {
        printf(" |> %s", cmd_show(*cmd));
    });

    if (chain.output_filepath) {
        printf(" |> %s", chain.output_filepath);
    }

    printf("\n");
}

int path_exists(Cstr path)
{
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributes(path);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES);
#else
    struct stat statbuf = {0};
    if (stat(path, &statbuf) < 0) {
        if (errno == ENOENT) {
            errno = 0;
            return 0;
        }

        PANIC("could not retrieve information about file %s: %s",
              path, strerror(errno));
    }

    return 1;
#endif
}

int path_is_dir(Cstr path)
{
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributes(path);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
            (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat statbuf = {0};
    if (stat(path, &statbuf) < 0) {
        if (errno == ENOENT) {
            errno = 0;
            return 0;
        }

        PANIC("could not retrieve information about file %s: %s",
              path, strerror(errno));
    }

    return S_ISDIR(statbuf.st_mode);
#endif // _WIN32
}

void path_rename(const char *old_path, const char *new_path)
{
#ifdef _WIN32
    if (!MoveFileEx(old_path, new_path, MOVEFILE_REPLACE_EXISTING)) {
        PANIC("could not rename %s to %s: %s", old_path, new_path,
              GetLastErrorAsString());
    }
#else
    if (rename(old_path, new_path) < 0) {
        PANIC("could not rename %s to %s: %s", old_path, new_path,
              strerror(errno));
    }
#endif // _WIN32
}

void path_mkdirs(Cstr_Array path)
{
    if (path.count == 0) {
        return;
    }

    size_t len = 0;
    for (size_t i = 0; i < path.count; ++i) {
        len += strlen(path.elems[i]);
    }

    size_t seps_count = path.count - 1;
    const size_t sep_len = strlen(PATH_SEP);

    char *result = malloc(len + seps_count * sep_len + 1);

    len = 0;
    for (size_t i = 0; i < path.count; ++i) {
        size_t n = strlen(path.elems[i]);
        memcpy(result + len, path.elems[i], n);
        len += n;

        if (seps_count > 0) {
            memcpy(result + len, PATH_SEP, sep_len);
            len += sep_len;
            seps_count -= 1;
        }

        result[len] = '\0';

        if (mkdir(result, 0755) < 0) {
            if (errno == EEXIST) {
                errno = 0;
                WARN("directory %s already exists", result);
            } else {
                PANIC("could not create directory %s: %s", result, strerror(errno));
            }
        }
    }
}

void path_rm(Cstr path)
{
    if (IS_DIR(path)) {
        FOREACH_FILE_IN_DIR(file, path, {
            if (strcmp(file, ".") != 0 && strcmp(file, "..") != 0)
            {
                path_rm(PATH(path, file));
            }
        });

        if (rmdir(path) < 0) {
            if (errno == ENOENT) {
                errno = 0;
                WARN("directory %s does not exist", path);
            } else {
                PANIC("could not remove directory %s: %s", path, strerror(errno));
            }
        }
    } else {
        if (unlink(path) < 0) {
            if (errno == ENOENT) {
                errno = 0;
                WARN("file %s does not exist", path);
            } else {
                PANIC("could not remove file %s: %s", path, strerror(errno));
            }
        }
    }
}

int is_path1_modified_after_path2(const char *path1, const char *path2)
{
#ifdef _WIN32
    FILETIME path1_time, path2_time;

    Fd path1_fd = fd_open_for_read(path1);
    if (!GetFileTime(path1_fd, NULL, NULL, &path1_time)) {
        PANIC("could not get time of %s: %s", path1, GetLastErrorAsString());
    }
    fd_close(path1_fd);

    Fd path2_fd = fd_open_for_read(path2);
    if (!GetFileTime(path2_fd, NULL, NULL, &path2_time)) {
        PANIC("could not get time of %s: %s", path2, GetLastErrorAsString());
    }
    fd_close(path2_fd);

    return CompareFileTime(&path1_time, &path2_time) == 1;
#else
    struct stat statbuf = {0};

    if (stat(path1, &statbuf) < 0) {
        PANIC("could not stat %s: %s\n", path1, strerror(errno));
    }
    int path1_time = statbuf.st_mtime;

    if (stat(path2, &statbuf) < 0) {
        PANIC("could not stat %s: %s\n", path2, strerror(errno));
    }
    int path2_time = statbuf.st_mtime;

    return path1_time > path2_time;
#endif
}

void VLOG(FILE *stream, Cstr tag, Cstr fmt, va_list args)
{
    fprintf(stream, "[%s] ", tag);
    vfprintf(stream, fmt, args);
    fprintf(stream, "\n");
}

void INFO(Cstr fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    VLOG(stderr, "INFO", fmt, args);
    va_end(args);
}

void WARN(Cstr fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    VLOG(stderr, "WARN", fmt, args);
    va_end(args);
}

void ERRO(Cstr fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    VLOG(stderr, "ERRO", fmt, args);
    va_end(args);
}

void PANIC(Cstr fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    VLOG(stderr, "ERRO", fmt, args);
    va_end(args);
    exit(1);
}

char *shift_args(int *argc, char ***argv)
{
    assert(*argc > 0);
    char *result = **argv;
    *argc -= 1;
    *argv += 1;
    return result;
}

//
// Author:   Jonathan Blow
// Version:  1
// Date:     31 August, 2018
//
// This code is released under the MIT license, which you can find at
//
//          https://opensource.org/licenses/MIT
//
//
//
// See the comments for how to use this library just below the includes.
//


//
// NOTE(Kalinovcic): I have translated the original implementation to C,
// and made a preprocessor define that enables people to include it without
// the implementation, just as a header.
//
// I also fixed two bugs:
//   - If COM initialization for VS2017 fails, we actually properly continue
//     searching for earlier versions in the registry.
//   - For earlier versions, the code now returns the "bin\amd64" VS directory.
//     Previously it was returning the "bin" directory, which is for x86 stuff.
//
// To include the implementation, before including microsoft_craziness.h, define:
//
//

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MICROSOFT_CRAZINESS_HEADER_GUARD
#define MICROSOFT_CRAZINESS_HEADER_GUARD

#include <string.h>

#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "OleAut32.lib")

typedef struct
{
    wchar_t *vs_exe_path;
    wchar_t *vs_library_path;
} Find_Result;

Find_Result find_visual_studio();
void free_resources(Find_Result *result);

#endif  // MICROSOFT_CRAZINESS_HEADER_GUARD


//
// HOW TO USE THIS CODE
//
// The purpose of this file is to find the folders that contain libraries
// you may need to link against, on Windows, if you are linking with any
// compiled C or C++ code. This will be necessary for many non-C++ programming
// language environments that want to provide compatibility.
//
// We find the place where the Visual Studio libraries live (for example,
// libvcruntime.lib), where the linker and compiler executables live
// (for example, link.exe), and where the Windows SDK libraries reside
// (kernel32.lib, libucrt.lib).
//
// We all wish you didn't have to worry about so many weird dependencies,
// but we don't really have a choice about this, sadly.
//
// I don't claim that this is the absolute best way to solve this problem,
// and so far we punt on things (if you have multiple versions of Visual Studio
// installed, we return the first one, rather than the newest). But it
// will solve the basic problem for you as simply as I know how to do it,
// and because there isn't too much code here, it's easy to modify and expand.
//
//
// Here is the API you need to know about:
//


//
// Call find_visual_studio_and_windows_sdk, look at the resulting
// paths, then call free_resources on the result.
//
// Everything else in this file is implementation details that you
// don't need to care about.
//

//
// This file was about 400 lines before we started adding these comments.
// You might think that's way too much code to do something as simple
// as finding a few library and executable paths. I agree. However,
// Microsoft's own solution to this problem, called "vswhere", is a
// mere EIGHT THOUSAND LINE PROGRAM, spread across 70 files,
// that they posted to github *unironically*.
//
// I am not making this up: https://github.com/Microsoft/vswhere
//
// Several people have therefore found the need to solve this problem
// themselves. We referred to some of these other solutions when
// figuring out what to do, most prominently ziglang's version,
// by Ryan Saunderson.
//
// I hate this kind of code. The fact that we have to do this at all
// is stupid, and the actual maneuvers we need to go through
// are just painful. If programming were like this all the time,
// I would quit.
//
// Because this is such an absurd waste of time, I felt it would be
// useful to package the code in an easily-reusable way, in the
// style of the stb libraries. We haven't gone as all-out as some
// of the stb libraries do (which compile in C with no includes, often).
// For this version you need C++ and the headers at the top of the file.
//
// We return the strings as Windows wide character strings. Aesthetically
// I don't like that (I think most sane programs are UTF-8 internally),
// but apparently, not all valid Windows file paths can even be converted
// correctly to UTF-8. So have fun with that. It felt safest and simplest
// to stay with wchar_t since all of this code is fully ensconced in
// Windows crazy-land.
//
// One other shortcut I took is that this is hardcoded to return the
// folders for x64 libraries. If you want x86 or arm, you can make
// slight edits to the code below, or, if enough people want this,
// I can work it in here.
//


#ifndef MICROSOFT_CRAZINESS_IMPLEMENTATION_GUARD
#define MICROSOFT_CRAZINESS_IMPLEMENTATION_GUARD


#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <io.h>         // For _get_osfhandle


void free_resources(Find_Result *result) {
    free(result->vs_exe_path);
    free(result->vs_library_path);
}


// COM objects for the ridiculous Microsoft craziness.

#undef  INTERFACE
#define INTERFACE ISetupInstance
DECLARE_INTERFACE_ (ISetupInstance, IUnknown)
{
    BEGIN_INTERFACE

    // IUnknown methods
    STDMETHOD (QueryInterface)   (THIS_  REFIID, void **) PURE;
    STDMETHOD_(ULONG, AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG, Release)   (THIS) PURE;

    // ISetupInstance methods
    STDMETHOD(GetInstanceId)(THIS_ _Out_ BSTR* pbstrInstanceId) PURE;
    STDMETHOD(GetInstallDate)(THIS_ _Out_ LPFILETIME pInstallDate) PURE;
    STDMETHOD(GetInstallationName)(THIS_ _Out_ BSTR* pbstrInstallationName) PURE;
    STDMETHOD(GetInstallationPath)(THIS_ _Out_ BSTR* pbstrInstallationPath) PURE;
    STDMETHOD(GetInstallationVersion)(THIS_ _Out_ BSTR* pbstrInstallationVersion) PURE;
    STDMETHOD(GetDisplayName)(THIS_ _In_ LCID lcid, _Out_ BSTR* pbstrDisplayName) PURE;
    STDMETHOD(GetDescription)(THIS_ _In_ LCID lcid, _Out_ BSTR* pbstrDescription) PURE;
    STDMETHOD(ResolvePath)(THIS_ _In_opt_z_ LPCOLESTR pwszRelativePath, _Out_ BSTR* pbstrAbsolutePath) PURE;

    END_INTERFACE
};

#undef  INTERFACE
#define INTERFACE IEnumSetupInstances
DECLARE_INTERFACE_ (IEnumSetupInstances, IUnknown)
{
    BEGIN_INTERFACE

    // IUnknown methods
    STDMETHOD (QueryInterface)   (THIS_  REFIID, void **) PURE;
    STDMETHOD_(ULONG, AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG, Release)   (THIS) PURE;

    // IEnumSetupInstances methods
    STDMETHOD(Next)(THIS_ _In_ ULONG celt, _Out_writes_to_(celt, *pceltFetched) ISetupInstance** rgelt, _Out_opt_ _Deref_out_range_(0, celt) ULONG* pceltFetched) PURE;
    STDMETHOD(Skip)(THIS_ _In_ ULONG celt) PURE;
    STDMETHOD(Reset)(THIS) PURE;
    STDMETHOD(Clone)(THIS_ _Deref_out_opt_ IEnumSetupInstances** ppenum) PURE;

    END_INTERFACE
};

#undef  INTERFACE
#define INTERFACE ISetupConfiguration
DECLARE_INTERFACE_ (ISetupConfiguration, IUnknown)
{
    BEGIN_INTERFACE

    // IUnknown methods
    STDMETHOD (QueryInterface)   (THIS_  REFIID, void **) PURE;
    STDMETHOD_(ULONG, AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG, Release)   (THIS) PURE;

    // ISetupConfiguration methods
    STDMETHOD(EnumInstances)(THIS_ _Out_ IEnumSetupInstances** ppEnumInstances) PURE;
    STDMETHOD(GetInstanceForCurrentProcess)(THIS_ _Out_ ISetupInstance** ppInstance) PURE;
    STDMETHOD(GetInstanceForPath)(THIS_ _In_z_ LPCWSTR wzPath, _Out_ ISetupInstance** ppInstance) PURE;

    END_INTERFACE
};

#ifdef __cplusplus
#define CALL_STDMETHOD(object, method, ...) object->method(__VA_ARGS__)
#define CALL_STDMETHOD_(object, method) object->method()
#else
#define CALL_STDMETHOD(object, method, ...) object->lpVtbl->method(object, __VA_ARGS__)
#define CALL_STDMETHOD_(object, method) object->lpVtbl->method(object)
#endif


// The beginning of the actual code that does things.

typedef struct {
    int32_t best_version[4];  // For Windows 8 versions, only two of these numbers are used.
    wchar_t *best_name;
} Version_Data;

bool os_file_exists(wchar_t *name) {
    // @Robustness: What flags do we really want to check here?

    auto attrib = GetFileAttributesW(name);
    if (attrib == INVALID_FILE_ATTRIBUTES) return false;
    if (attrib & FILE_ATTRIBUTE_DIRECTORY) return false;

    return true;
}

#define concat2(a, b) concat(a, b, NULL, NULL, NULL)
#define concat3(a, b, c) concat(a, b, c, NULL, NULL)
#define concat4(a, b, c, d) concat(a, b, c, d, NULL)
#define concat5(a, b, c, d, e) concat(a, b, c, d, e)
wchar_t *concat(wchar_t *a, wchar_t *b, wchar_t *c, wchar_t *d, wchar_t *e) {
    // Concatenate up to 4 wide strings together. Allocated with malloc.
    // If you don't like that, use a programming language that actually
    // helps you with using custom allocators. Or just edit the code.

    auto len_a = wcslen(a);
    auto len_b = wcslen(b);

    auto len_c = 0;
    if (c) len_c = wcslen(c);

    auto len_d = 0;
    if (d) len_d = wcslen(d);

    auto len_e = 0;
    if (e) len_e = wcslen(e);

    wchar_t *result = (wchar_t *)malloc((len_a + len_b + len_c + len_d + len_e + 1) * 2);
    memcpy(result, a, len_a*2);
    memcpy(result + len_a, b, len_b*2);

    if (c) memcpy(result + len_a + len_b, c, len_c * 2);
    if (d) memcpy(result + len_a + len_b + len_c, d, len_d * 2);
    if (e) memcpy(result + len_a + len_b + len_c + len_d, e, len_e * 2);

    result[len_a + len_b + len_c + len_d + len_e] = 0;

    return result;
}

typedef void (*Visit_Proc_W)(wchar_t *short_name, wchar_t *full_name, Version_Data *data);
bool visit_files_w(wchar_t *dir_name, Version_Data *data, Visit_Proc_W proc) {

    // Visit everything in one folder (non-recursively). If it's a directory
    // that doesn't start with ".", call the visit proc on it. The visit proc
    // will see if the filename conforms to the expected versioning pattern.

    WIN32_FIND_DATAW find_data;

    wchar_t *wildcard_name = concat2(dir_name, L"\\*");
    HANDLE handle = FindFirstFileW(wildcard_name, &find_data);
    free(wildcard_name);

    if (handle == INVALID_HANDLE_VALUE) return false;

    while (true) {
        if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (find_data.cFileName[0] != '.')) {
            wchar_t *full_name = concat3(dir_name, L"\\", find_data.cFileName);
            proc(find_data.cFileName, full_name, data);
            free(full_name);
        }

        BOOL success = FindNextFileW(handle, &find_data);
        if (!success) break;
    }

    FindClose(handle);

    return true;
}


void win10_best(wchar_t *short_name, wchar_t *full_name, Version_Data *data) {
    // Find the Windows 10 subdirectory with the highest version number.

    int i0, i1, i2, i3;
    auto success = swscanf_s(short_name, L"%d.%d.%d.%d", &i0, &i1, &i2, &i3);
    if (success < 4) return;

    if (i0 < data->best_version[0]) return;
    else if (i0 == data->best_version[0]) {
        if (i1 < data->best_version[1]) return;
        else if (i1 == data->best_version[1]) {
            if (i2 < data->best_version[2]) return;
            else if (i2 == data->best_version[2]) {
                if (i3 < data->best_version[3]) return;
            }
        }
    }

    // we have to copy_string and free here because visit_files free's the full_name string
    // after we execute this function, so Win*_Data would contain an invalid pointer.
    if (data->best_name) free(data->best_name);
    data->best_name = _wcsdup(full_name);

    if (data->best_name) {
        data->best_version[0] = i0;
        data->best_version[1] = i1;
        data->best_version[2] = i2;
        data->best_version[3] = i3;
    }
}

void win8_best(wchar_t *short_name, wchar_t *full_name, Version_Data *data) {
    // Find the Windows 8 subdirectory with the highest version number.

    int i0, i1;
    auto success = swscanf_s(short_name, L"winv%d.%d", &i0, &i1);
    if (success < 2) return;

    if (i0 < data->best_version[0]) return;
    else if (i0 == data->best_version[0]) {
        if (i1 < data->best_version[1]) return;
    }

    // we have to copy_string and free here because visit_files free's the full_name string
    // after we execute this function, so Win*_Data would contain an invalid pointer.
    if (data->best_name) free(data->best_name);
    data->best_name = _wcsdup(full_name);

    if (data->best_name) {
        data->best_version[0] = i0;
        data->best_version[1] = i1;
    }
}

bool find_visual_studio_2017_by_fighting_through_microsoft_craziness(Find_Result *result) {
    HRESULT rc = CoInitialize(NULL);
    // "Subsequent valid calls return false." So ignore false.
    // if rc != S_OK  return false;

    GUID my_uid                   = {0x42843719, 0xDB4C, 0x46C2, {0x8E, 0x7C, 0x64, 0xF1, 0x81, 0x6E, 0xFD, 0x5B}};
    GUID CLSID_SetupConfiguration = {0x177F0C4A, 0x1CD3, 0x4DE7, {0xA3, 0x2C, 0x71, 0xDB, 0xBB, 0x9F, 0xA3, 0x6D}};

    ISetupConfiguration *config = NULL;

    // NOTE(Kalinovcic): This is so stupid... These functions take references, so the code is different for C and C++......
#ifdef __cplusplus
    HRESULT hr = CoCreateInstance(CLSID_SetupConfiguration, NULL, CLSCTX_INPROC_SERVER, my_uid, (void **)&config);
#else
    HRESULT hr = CoCreateInstance(&CLSID_SetupConfiguration, NULL, CLSCTX_INPROC_SERVER, &my_uid, (void **)&config);
#endif

    if (hr != 0)  return false;

    IEnumSetupInstances *instances = NULL;
    hr = CALL_STDMETHOD(config, EnumInstances, &instances);
    CALL_STDMETHOD_(config, Release);
    if (hr != 0)     return false;
    if (!instances)  return false;

    bool found_visual_studio_2017 = false;
    while (1) {
        ULONG found = 0;
        ISetupInstance *instance = NULL;
        HRESULT hr = CALL_STDMETHOD(instances, Next, 1, &instance, &found);
        if (hr != S_OK) break;

        BSTR bstr_inst_path;
        hr = CALL_STDMETHOD(instance, GetInstallationPath, &bstr_inst_path);
        CALL_STDMETHOD_(instance, Release);
        if (hr != S_OK)  continue;

        wchar_t *tools_filename = concat2(bstr_inst_path, L"\\VC\\Auxiliary\\Build\\Microsoft.VCToolsVersion.default.txt");
        SysFreeString(bstr_inst_path);

        FILE *f;
        errno_t open_result = _wfopen_s(&f, tools_filename, L"rt");
        free(tools_filename);
        if (open_result != 0) continue;
        if (!f) continue;

        LARGE_INTEGER tools_file_size;
        HANDLE file_handle = (HANDLE)_get_osfhandle(_fileno(f));
        BOOL success = GetFileSizeEx(file_handle, &tools_file_size);
        if (!success) {
            fclose(f);
            continue;
        }

        uint64_t version_bytes = (tools_file_size.QuadPart + 1) * 2;  // Warning: This multiplication by 2 presumes there is no variable-length encoding in the wchars (wacky characters in the file could betray this expectation).
        wchar_t *version = (wchar_t *)malloc(version_bytes);

        wchar_t *read_result = fgetws(version, version_bytes, f);
        fclose(f);
        if (!read_result) continue;

        wchar_t *version_tail = wcschr(version, '\n');
        if (version_tail)  *version_tail = 0;  // Stomp the data, because nobody cares about it.

        wchar_t *library_path = concat4(bstr_inst_path, L"\\VC\\Tools\\MSVC\\", version, L"\\lib\\x64");
        wchar_t *library_file = concat2(library_path, L"\\vcruntime.lib");  // @Speed: Could have library_path point to this string, with a smaller count, to save on memory flailing!

        if (os_file_exists(library_file)) {
            free(version);
            wchar_t *vcvarsall = concat2(bstr_inst_path, L"\\VC\\Auxiliary\\Build\\vcvarsall.bat");
            result->vs_exe_path     = vcvarsall;
            result->vs_library_path = library_path;
            found_visual_studio_2017 = true;
            break;
        }

        free(version);

        /*
           Ryan Saunderson said:
           "Clang uses the 'SetupInstance->GetInstallationVersion' / ISetupHelper->ParseVersion to find the newest version
           and then reads the tools file to define the tools path - which is definitely better than what i did."

           So... @Incomplete: Should probably pick the newest version...
        */
    }

    CALL_STDMETHOD_(instances, Release);
    return found_visual_studio_2017;
}

void find_visual_studio_by_fighting_through_microsoft_craziness(Find_Result *result) {
    // The name of this procedure is kind of cryptic. Its purpose is
    // to fight through Microsoft craziness. The things that the fine
    // Visual Studio team want you to do, JUST TO FIND A SINGLE FOLDER
    // THAT EVERYONE NEEDS TO FIND, are ridiculous garbage.

    // For earlier versions of Visual Studio, you'd find this information in the registry,
    // similarly to the Windows Kits above. But no, now it's the future, so to ask the
    // question "Where is the Visual Studio folder?" you have to do a bunch of COM object
    // instantiation, enumeration, and querying. (For extra bonus points, try doing this in
    // a new, underdeveloped programming language where you don't have COM routines up
    // and running yet. So fun.)
    //
    // If all this COM object instantiation, enumeration, and querying doesn't give us
    // a useful result, we drop back to the registry-checking method.

    bool found_visual_studio_2017 = find_visual_studio_2017_by_fighting_through_microsoft_craziness(result);
    if (found_visual_studio_2017)  return;


    // If we get here, we didn't find Visual Studio 2017. Try earlier versions.

    HKEY vs7_key;
    HRESULT rc = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VS7", 0, KEY_QUERY_VALUE | KEY_WOW64_32KEY, &vs7_key);
    if (rc != S_OK)  return;

    // Hardcoded search for 4 prior Visual Studio versions. Is there something better to do here?
    wchar_t *versions[] = { L"14.0", L"12.0", L"11.0", L"10.0" };
    const int NUM_VERSIONS = sizeof(versions) / sizeof(versions[0]);

    for (int i = 0; i < NUM_VERSIONS; i++) {
        wchar_t *v = versions[i];

        DWORD dw_type;
        DWORD cb_data;

        LSTATUS rc = RegQueryValueExW(vs7_key, v, NULL, &dw_type, NULL, &cb_data);
        if ((rc == ERROR_FILE_NOT_FOUND) || (dw_type != REG_SZ)) {
            continue;
        }

        wchar_t *buffer = (wchar_t *)malloc(cb_data);
        if (!buffer)  return;

        rc = RegQueryValueExW(vs7_key, v, NULL, NULL, (LPBYTE)buffer, &cb_data);
        if (rc != 0)  continue;

        // @Robustness: Do the zero-termination thing suggested in the RegQueryValue docs?

        wchar_t *lib_path = concat2(buffer, L"VC\\Lib\\amd64");

        // Check to see whether a vcruntime.lib actually exists here.
        wchar_t *vcruntime_filename = concat2(lib_path, L"\\vcruntime.lib");
        bool vcruntime_exists = os_file_exists(vcruntime_filename);
        free(vcruntime_filename);

        if (vcruntime_exists) {
            // result->vs_exe_path     = concat2(buffer, L"VC\\bin\\amd64");
            // result->vs_library_path = lib_path;

            free(buffer);
            RegCloseKey(vs7_key);
            return;
        }

        free(lib_path);
        free(buffer);
    }

    RegCloseKey(vs7_key);

    // If we get here, we failed to find anything.
}


Find_Result find_visual_studio() {
    Find_Result result;
    result.vs_exe_path = 0;
    result.vs_library_path = 0;
    find_visual_studio_by_fighting_through_microsoft_craziness(&result);
    return result;
}


#endif  // MICROSOFT_CRAZINESS_IMPLEMENTATION_GUARD

#ifdef __cplusplus
}
#endif

// dear imgui
// (binary_to_compressed_c.cpp)
// Helper tool to turn a file into a C array, if you want to embed font data in your source code.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

// stb_compress* from stb.h - declaration
typedef unsigned int stb_uint;
typedef unsigned char stb_uchar;
stb_uint stb_compress(stb_uchar* out, stb_uchar* in, stb_uint len);

static bool binary_to_compressed_c(const char* filename, const char* out_filename, const char* symbol, bool use_base85_encoding, bool use_compression, bool use_static);

char Encode85Byte(unsigned int x)
{
    x = (x % 85) + 35;
    return (char)((x >= '\\') ? x + 1 : x);
}

bool binary_to_compressed_c(const char* filename, const char* out_filename, const char* symbol, bool use_base85_encoding, bool use_compression, bool use_static)
{
    // Read file
    FILE* f = fopen(filename, "rb");
    if (!f) return false;
    int data_sz;
    if (fseek(f, 0, SEEK_END) || (data_sz = (int)ftell(f)) == -1 || fseek(f, 0, SEEK_SET)) { fclose(f); return false; }
    char* data = calloc(1, data_sz + 4);
    if (fread(data, 1, data_sz, f) != (size_t)data_sz) { fclose(f); free(data); return false; }
    memset((void*)(((char*)data) + data_sz), 0, 4);
    fclose(f);

    // Compress
    int maxlen = data_sz + 512 + (data_sz >> 2) + sizeof(int); // total guess
    char* compressed = use_compression ? calloc(1, maxlen) : data;
    int compressed_sz = use_compression ? stb_compress((stb_uchar*)compressed, (stb_uchar*)data, data_sz) : data_sz;
    if (use_compression)
        memset(compressed + compressed_sz, 0, maxlen - compressed_sz);

    // Output as Base85 encoded
    FILE* out = fopen(out_filename, "wb");
    fprintf(out, "// File: '%s' (%d bytes)\n", filename, (int)data_sz);
    fprintf(out, "// Exported using binary_to_compressed_c.cpp\n");
    const char* static_str = use_static ? "static " : "";
    const char* compressed_str = use_compression ? "compressed_" : "";
    if (use_base85_encoding)
    {
        fprintf(out, "%sconst char %s_%sdata_base85[%d+1] =\n    \"", static_str, symbol, compressed_str, (int)((compressed_sz + 3) / 4)*5);
        char prev_c = 0;
        for (int src_i = 0; src_i < compressed_sz; src_i += 4)
        {
            // This is made a little more complicated by the fact that ??X sequences are interpreted as trigraphs by old C/C++ compilers. So we need to escape pairs of ??.
            unsigned int d = *(unsigned int*)(compressed + src_i);
            for (unsigned int n5 = 0; n5 < 5; n5++, d /= 85)
            {
                char c = Encode85Byte(d);
                fprintf(out, (c == '?' && prev_c == '?') ? "\\%c" : "%c", c);
                prev_c = c;
            }
            if ((src_i % 112) == 112 - 4)
                fprintf(out, "\"\n    \"");
        }
        fprintf(out, "\";\n\n");
    }
    else
    {
        fprintf(out, "%sconst unsigned int %s_%ssize = %d;\n", static_str, symbol, compressed_str, (int)compressed_sz);
        fprintf(out, "%sconst unsigned int %s_%sdata[%d/4] =\n{", static_str, symbol, compressed_str, (int)((compressed_sz + 3) / 4)*4);
        int column = 0;
        for (int i = 0; i < compressed_sz; i += 4)
        {
            unsigned int d = *(unsigned int*)(compressed + i);
            if ((column++ % 12) == 0)
                fprintf(out, "\n    0x%08x, ", d);
            else
                fprintf(out, "0x%08x, ", d);
        }
        fprintf(out, "\n};\n\n");
    }

    // Cleanup
    fclose(out);
    free(data);
    if (use_compression)
        free(compressed);
    return true;
}

// stb_compress* from stb.h - definition

////////////////////           compressor         ///////////////////////

static stb_uint stb_adler32(stb_uint adler32, stb_uchar *buffer, stb_uint buflen)
{
    const unsigned long ADLER_MOD = 65521;
    unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long blocklen, i;

    blocklen = buflen % 5552;
    while (buflen) {
        for (i=0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0], s2 += s1;
            s1 += buffer[1], s2 += s1;
            s1 += buffer[2], s2 += s1;
            s1 += buffer[3], s2 += s1;
            s1 += buffer[4], s2 += s1;
            s1 += buffer[5], s2 += s1;
            s1 += buffer[6], s2 += s1;
            s1 += buffer[7], s2 += s1;

            buffer += 8;
        }

        for (; i < blocklen; ++i)
            s1 += *buffer++, s2 += s1;

        s1 %= ADLER_MOD, s2 %= ADLER_MOD;
        buflen -= blocklen;
        blocklen = 5552;
    }
    return (s2 << 16) + s1;
}

static unsigned int stb_matchlen(stb_uchar *m1, stb_uchar *m2, stb_uint maxlen)
{
    stb_uint i;
    for (i=0; i < maxlen; ++i)
        if (m1[i] != m2[i]) return i;
    return i;
}

// simple implementation that just takes the source data in a big block

static stb_uchar *stb__out;
static FILE      *stb__outfile;
static stb_uint   stb__outbytes;

static void stb__write(unsigned char v)
{
    fputc(v, stb__outfile);
    ++stb__outbytes;
}

//#define stb_out(v)    (stb__out ? *stb__out++ = (stb_uchar) (v) : stb__write((stb_uchar) (v)))
#define stb_out(v)    do { if (stb__out) *stb__out++ = (stb_uchar) (v); else stb__write((stb_uchar) (v)); } while (0)

static void stb_out2(stb_uint v) { stb_out(v >> 8); stb_out(v); }
static void stb_out3(stb_uint v) { stb_out(v >> 16); stb_out(v >> 8); stb_out(v); }
static void stb_out4(stb_uint v) { stb_out(v >> 24); stb_out(v >> 16); stb_out(v >> 8 ); stb_out(v); }

static void outliterals(stb_uchar *in, int numlit)
{
    while (numlit > 65536) {
        outliterals(in,65536);
        in     += 65536;
        numlit -= 65536;
    }

    if      (numlit ==     0)    ;
    else if (numlit <=    32)    stb_out (0x000020 + numlit-1);
    else if (numlit <=  2048)    stb_out2(0x000800 + numlit-1);
    else /*  numlit <= 65536) */ stb_out3(0x070000 + numlit-1);

    if (stb__out) {
        memcpy(stb__out,in,numlit);
        stb__out += numlit;
    } else
        fwrite(in, 1, numlit, stb__outfile);
}

static int stb__window = 0x40000; // 256K

static int stb_not_crap(int best, int dist)
{
    return   ((best > 2  &&  dist <= 0x00100)
        || (best > 5  &&  dist <= 0x04000)
        || (best > 7  &&  dist <= 0x80000));
}

static  stb_uint stb__hashsize = 32768;

// note that you can play with the hashing functions all you
// want without needing to change the decompressor
#define stb__hc(q,h,c)      (((h) << 7) + ((h) >> 25) + q[c])
#define stb__hc2(q,h,c,d)   (((h) << 14) + ((h) >> 18) + (q[c] << 7) + q[d])
#define stb__hc3(q,c,d,e)   ((q[c] << 14) + (q[d] << 7) + q[e])

static unsigned int stb__running_adler;

static int stb_compress_chunk(stb_uchar *history,
    stb_uchar *start,
    stb_uchar *end,
    int length,
    int *pending_literals,
    stb_uchar **chash,
    stb_uint mask)
{
    (void)history;
    int window = stb__window;
    stb_uint match_max;
    stb_uchar *lit_start = start - *pending_literals;
    stb_uchar *q = start;

#define STB__SCRAMBLE(h)   (((h) + ((h) >> 16)) & mask)

    // stop short of the end so we don't scan off the end doing
    // the hashing; this means we won't compress the last few bytes
    // unless they were part of something longer
    while (q < start+length && q+12 < end) {
        int m;
        stb_uint h1,h2,h3,h4, h;
        stb_uchar *t;
        int best = 2, dist=0;

        if (q+65536 > end)
            match_max = (stb_uint)(end-q);
        else
            match_max = 65536;

#define stb__nc(b,d)  ((d) <= window && ((b) > 9 || stb_not_crap((int)(b),(int)(d))))

#define STB__TRY(t,p)  /* avoid retrying a match we already tried */ \
    if (p ? dist != (int)(q-t) : 1)                             \
    if ((m = stb_matchlen(t, q, match_max)) > best)     \
    if (stb__nc(m,q-(t)))                                \
    best = m, dist = (int)(q - (t))

        // rather than search for all matches, only try 4 candidate locations,
        // chosen based on 4 different hash functions of different lengths.
        // this strategy is inspired by LZO; hashing is unrolled here using the
        // 'hc' macro
        h = stb__hc3(q,0, 1, 2); h1 = STB__SCRAMBLE(h);
        t = chash[h1]; if (t) STB__TRY(t,0);
        h = stb__hc2(q,h, 3, 4); h2 = STB__SCRAMBLE(h);
        h = stb__hc2(q,h, 5, 6);        t = chash[h2]; if (t) STB__TRY(t,1);
        h = stb__hc2(q,h, 7, 8); h3 = STB__SCRAMBLE(h);
        h = stb__hc2(q,h, 9,10);        t = chash[h3]; if (t) STB__TRY(t,1);
        h = stb__hc2(q,h,11,12); h4 = STB__SCRAMBLE(h);
        t = chash[h4]; if (t) STB__TRY(t,1);

        // because we use a shared hash table, can only update it
        // _after_ we've probed all of them
        chash[h1] = chash[h2] = chash[h3] = chash[h4] = q;

        if (best > 2)
            assert(dist > 0);

        // see if our best match qualifies
        if (best < 3) { // fast path literals
            ++q;
        } else if (best > 2  &&  best <= 0x80    &&  dist <= 0x100) {
            outliterals(lit_start, (int)(q-lit_start)); lit_start = (q += best);
            stb_out(0x80 + best-1);
            stb_out(dist-1);
        } else if (best > 5  &&  best <= 0x100   &&  dist <= 0x4000) {
            outliterals(lit_start, (int)(q-lit_start)); lit_start = (q += best);
            stb_out2(0x4000 + dist-1);
            stb_out(best-1);
        } else if (best > 7  &&  best <= 0x100   &&  dist <= 0x80000) {
            outliterals(lit_start, (int)(q-lit_start)); lit_start = (q += best);
            stb_out3(0x180000 + dist-1);
            stb_out(best-1);
        } else if (best > 8  &&  best <= 0x10000 &&  dist <= 0x80000) {
            outliterals(lit_start, (int)(q-lit_start)); lit_start = (q += best);
            stb_out3(0x100000 + dist-1);
            stb_out2(best-1);
        } else if (best > 9                      &&  dist <= 0x1000000) {
            if (best > 65536) best = 65536;
            outliterals(lit_start, (int)(q-lit_start)); lit_start = (q += best);
            if (best <= 0x100) {
                stb_out(0x06);
                stb_out3(dist-1);
                stb_out(best-1);
            } else {
                stb_out(0x04);
                stb_out3(dist-1);
                stb_out2(best-1);
            }
        } else {  // fallback literals if no match was a balanced tradeoff
            ++q;
        }
    }

    // if we didn't get all the way, add the rest to literals
    if (q-start < length)
        q = start+length;

    // the literals are everything from lit_start to q
    *pending_literals = (int)(q - lit_start);

    stb__running_adler = stb_adler32(stb__running_adler, start, (stb_uint)(q - start));
    return (int)(q - start);
}

static int stb_compress_inner(stb_uchar *input, stb_uint length)
{
    int literals = 0;
    stb_uint len,i;

    stb_uchar **chash;
    chash = (stb_uchar**) malloc(stb__hashsize * sizeof(stb_uchar*));
    if (chash == 0) return 0; // failure
    for (i=0; i < stb__hashsize; ++i)
        chash[i] = 0;

    // stream signature
    stb_out(0x57); stb_out(0xbc);
    stb_out2(0);

    stb_out4(0);       // 64-bit length requires 32-bit leading 0
    stb_out4(length);
    stb_out4(stb__window);

    stb__running_adler = 1;

    len = stb_compress_chunk(input, input, input+length, length, &literals, chash, stb__hashsize-1);
    assert(len == length);

    outliterals(input+length - literals, literals);

    free(chash);

    stb_out2(0x05fa); // end opcode

    stb_out4(stb__running_adler);

    return 1; // success
}

stb_uint stb_compress(stb_uchar *out, stb_uchar *input, stb_uint length)
{
    stb__out = out;
    stb__outfile = 0;

    stb_compress_inner(input, length);

    return (stb_uint)(stb__out - out);
}

// NOTE(Ciremun): flag.h
#define FLAGS_CAP 256

typedef struct
{
    char *name;
    int exists;
} Flag;

typedef struct
{
    Flag flags[FLAGS_CAP];
    size_t flags_count;
} Flag_Context;

Flag_Context flags_ctx = {0};

static int *flag_int(const char *name)
{
    Flag_Context *c = &flags_ctx;
    Flag *flag = &c->flags[c->flags_count++];
    memset(flag, 0, sizeof(*flag));
    flag->name = (char*)name;
    return &flag->exists;
}

static char *flag_shift_args(int *argc, char ***argv)
{
    assert(*argc > 0);
    char *result = **argv;
    *argv += 1;
    *argc -= 1;
    return result;
}

static void parse_flags(int argc, char **argv)
{
    Flag_Context *c = &flags_ctx;

    flag_shift_args(&argc, &argv);

    while (argc > 0) {
        char *flag = flag_shift_args(&argc, &argv);
        int found = 0;
        for (size_t i = 0; i < c->flags_count; ++i) {
            if (strcmp(c->flags[i].name, flag) == 0) {
                c->flags[i].exists = 1;
                found = 1;
            }
        }
        if (!found)
        {
            ERRO("unknown flag '%s'", flag);
            exit(1);
        }
    }
}

#endif // NOBUILD_IMPLEMENTATION
