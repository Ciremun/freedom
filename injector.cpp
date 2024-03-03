#include <windows.h>
#include <tlhelp32.h>

#include <string>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define mks(STRING) ([&] {                                       \
    constexpr auto _{ crypt(STRING, seed(__FILE__, __LINE__)) }; \
    return std::string{ crypt(_.data, _.seed).data };            \
}())

#define mkfunc(f) auto s##f(mks(#f)); _##f = (t##f)GetProcAddress(k32, s##f.c_str())

typedef DWORD (WINAPI *tGetFullPathNameW)(LPCWSTR lpFileName, DWORD nBufferLength, LPWSTR lpBuffer, LPWSTR *lpFilePart);
typedef DWORD (WINAPI *tGetLastError)();
typedef HANDLE (WINAPI *tCreateToolhelp32Snapshot)(DWORD dwFlags, DWORD th32ProcessID);
typedef BOOL (WINAPI *tProcess32FirstW)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef BOOL (WINAPI *tProcess32NextW)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef BOOL (WINAPI *tCloseHandle)(HANDLE hObject);
typedef HANDLE (WINAPI *tOpenProcess)(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId);
typedef LPVOID (WINAPI *tVirtualAllocEx)(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
typedef BOOL (WINAPI *tWriteProcessMemory)(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T *lpNumberOfBytesWritten);
typedef HANDLE (WINAPI *tCreateRemoteThread)(HANDLE hProcess, LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
typedef HMODULE (WINAPI *tLoadLibraryW)(LPCWSTR lpLibFileName);

tCreateToolhelp32Snapshot _CreateToolhelp32Snapshot = 0;
tGetFullPathNameW _GetFullPathNameW = 0;
tGetLastError _GetLastError = 0;
tProcess32FirstW _Process32FirstW = 0;
tProcess32NextW _Process32NextW = 0;
tCloseHandle _CloseHandle = 0;
tOpenProcess _OpenProcess = 0;
tVirtualAllocEx _VirtualAllocEx = 0;
tWriteProcessMemory _WriteProcessMemory = 0;
tCreateRemoteThread _CreateRemoteThread = 0;
tLoadLibraryW _LoadLibraryW = 0;

// https://gist.github.com/EvanMcBroom/ace2a9af19fb5e7b2451b1cd4c07bf96
constexpr uint32_t modulus() {
    return 0x7fffffff;
}

constexpr uint32_t prng(const uint32_t input) {
    return (input * 48271) % modulus();
}

template<size_t N>
constexpr uint32_t seed(const char(&entropy)[N], const uint32_t iv = 0) {
    auto value{ iv };
    for (size_t i{ 0 }; i < N; i++) {
        // Xor 1st byte of seed with input byte
        value = (value & ((~0) << 8)) | ((value & 0xFF) ^ entropy[i]);
        // Rotate left 1 byte
        value = value << 8 | value >> ((sizeof(value) * 8) - 8);
    }
    // The seed is required to be less than the modulus and odd
    while (value > modulus()) value = value >> 1;
    return value << 1 | 1;
}

template<typename T, size_t N>
struct encrypted {
    int seed;
    T data[N];
};

template<size_t N>
constexpr auto crypt(const char(&input)[N], const uint32_t seed = 0) {
    encrypted<char, N> blob{};
    blob.seed = seed;
    for (uint32_t index{ 0 }, stream{ seed }; index < N; index++) {
        blob.data[index] = input[index] ^ stream;
        stream = prng(stream);
    }
    return blob;
}

static inline DWORD get_process_id(const wchar_t *process_name)
{
    DWORD process_id = 0;
    HANDLE hSnap = _CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);

        if (_Process32FirstW(hSnap, &procEntry))
        {
            do
            {
                if (!_wcsicmp(procEntry.szExeFile, process_name))
                {
                    process_id = procEntry.th32ProcessID;
                    break;
                }
            } while (_Process32NextW(hSnap, &procEntry));
        }
    }
    _CloseHandle(hSnap);
    return process_id;
}

int wmain(int argc, wchar_t **argv, wchar_t **envp)
{
    auto sKernel32Dll(mks("Kernel32.dll"));
    auto k32 = GetModuleHandleA(sKernel32Dll.c_str());
    mkfunc(GetFullPathNameW);
    mkfunc(CreateToolhelp32Snapshot);
    mkfunc(Process32FirstW);
    mkfunc(Process32NextW);
    mkfunc(CloseHandle);
    mkfunc(OpenProcess);
    mkfunc(VirtualAllocEx);
    mkfunc(WriteProcessMemory);
    mkfunc(CreateRemoteThread);
    mkfunc(LoadLibraryW);
    mkfunc(GetLastError);

    auto get_process_id_err = mks("get_process_id failed: launch %S first!\n");
    auto getfullpathnamea_err = mks("GetFullPathNameA failed: %ld\n");
    auto createremotethread_err = mks("CreateRemoteThread failed: %ld\n");
    auto writeprocessmemory_err = mks("WriteProcessMemory failed: %ld\n");
    auto virtualallocex_err = mks("VirtualAllocEx failed: %ld\n");
    auto openprocess_err = mks("OpenProcess failed: %ld\n");

    auto process_name_s = mks("osu!.exe");
    auto process_name_w = std::wstring(process_name_s.begin(), process_name_s.end());

    auto dll_name_s = mks("freedom.dll");
    auto dll_name_w = std::wstring(dll_name_s.begin(), dll_name_s.end());

    const wchar_t *process_name = argc > 1 ? argv[1] : process_name_w.c_str();
    DWORD process_id = get_process_id(process_name);
    if (process_id == 0)
    {
        fprintf(stderr, get_process_id_err.c_str(), process_name);
        return 1;
    }

    const wchar_t *dll_name = argc > 2 ? argv[2] : dll_name_w.c_str();
    static wchar_t module_path[MAX_PATH * 2];
    DWORD module_path_length = _GetFullPathNameW(dll_name, MAX_PATH * 2, module_path, NULL);
    if (module_path_length == 0)
    {
        fprintf(stderr, getfullpathnamea_err.c_str(), _GetLastError());
        return 1;
    }

    HANDLE hProc = _OpenProcess(PROCESS_ALL_ACCESS, 0, process_id);
    if (hProc != NULL)
    {
        void *loc = _VirtualAllocEx(hProc, 0, module_path_length * sizeof(wchar_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (loc)
        {
            if (_WriteProcessMemory(hProc, loc, module_path, module_path_length * sizeof(wchar_t), 0))
            {
                HANDLE hThread = _CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)_LoadLibraryW, loc, 0, 0);
                if (hThread)
                    _CloseHandle(hThread);
                else
                    fprintf(stderr, createremotethread_err.c_str(), _GetLastError());
            }
            else
                fprintf(stderr, writeprocessmemory_err.c_str(), _GetLastError());
        }
        else
            fprintf(stderr, virtualallocex_err.c_str(), _GetLastError());
    }
    else
        fprintf(stderr, openprocess_err.c_str(), _GetLastError());

    if (hProc)
        _CloseHandle(hProc);

    return 0;
}
