// MIT License

// Copyright (c) 2021 TheCruZ

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <windows.h>
#include <tlhelp32.h>

#include <string>

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#ifdef _WIN64
#define CURRENT_ARCH IMAGE_FILE_MACHINE_AMD64
#else
#define CURRENT_ARCH IMAGE_FILE_MACHINE_I386
#endif // _WIN64

#define RELOC_FLAG32(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_HIGHLOW)
#define RELOC_FLAG64(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_DIR64)

#ifdef _WIN64
#define RELOC_FLAG RELOC_FLAG64
#else
#define RELOC_FLAG RELOC_FLAG32
#endif // _WIN64

#define log_info(fmt, ...) printf("INFO: " fmt "\n", __VA_ARGS__)
#define log_warn(fmt, ...) printf("WARN: " fmt "\n", __VA_ARGS__)
#define log_error(fmt, ...) printf("ERROR: " fmt "\n", __VA_ARGS__)

#define mks(STRING) ([&] {                                       \
    constexpr auto _{ crypt(STRING, seed(__FILE__, __LINE__)) }; \
    return std::string{ crypt(_.data, _.seed).data };            \
}())

#define k32func(f) auto s##f(mks(#f)); _##f = (t##f)GetProcAddress(k32, s##f.c_str()); assert(_##f != NULL)
#define a32func(f) auto s##f(mks(#f)); _##f = (t##f)GetProcAddress(a32, s##f.c_str()); assert(_##f != NULL)

typedef DWORD (WINAPI *tGetFullPathNameW)(LPCWSTR lpFileName, DWORD nBufferLength, LPWSTR lpBuffer, LPWSTR *lpFilePart);
typedef DWORD (WINAPI *tGetLastError)();
typedef HANDLE (WINAPI *tCreateToolhelp32Snapshot)(DWORD dwFlags, DWORD th32ProcessID);
typedef BOOL (WINAPI *tProcess32FirstW)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef BOOL (WINAPI *tProcess32NextW)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef BOOL (WINAPI *tCloseHandle)(HANDLE hObject);
typedef HANDLE (WINAPI *tOpenProcess)(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId);
typedef LPVOID (WINAPI *tVirtualAllocEx)(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
typedef BOOL (WINAPI *tVirtualProtectEx)(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);
typedef BOOL (WINAPI *tVirtualFreeEx)(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
typedef BOOL (WINAPI *tWriteProcessMemory)(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T *lpNumberOfBytesWritten);
typedef BOOL (WINAPI *tReadProcessMemory)(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T *lpNumberOfBytesRead);
typedef HMODULE (WINAPI *tLoadLibraryW)(LPCWSTR lpLibFileName);
typedef BOOL (WINAPI *tIsWow64Process)( HANDLE hProcess, PBOOL Wow64Process);
typedef BOOL (WINAPI *tOpenProcessToken)(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle);
typedef BOOL (WINAPI *tLookupPrivilegeValueW)(LPCWSTR lpSystemName, LPCWSTR lpName, PLUID lpLuid);
typedef HANDLE (WINAPI *tGetCurrentProcess)();
typedef HANDLE (WINAPI *tCreateRemoteThread)(HANDLE hProcess, LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize,
                                             LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
typedef BOOL (WINAPI *tAdjustTokenPrivileges)(HANDLE TokenHandle, BOOL DisableAllPrivileges, PTOKEN_PRIVILEGES NewState,
                                             DWORD BufferLength, PTOKEN_PRIVILEGES PreviousState, PDWORD ReturnLength);

tCreateToolhelp32Snapshot _CreateToolhelp32Snapshot = 0;
tGetFullPathNameW _GetFullPathNameW = 0;
tGetLastError _GetLastError = 0;
tProcess32FirstW _Process32FirstW = 0;
tProcess32NextW _Process32NextW = 0;
tCloseHandle _CloseHandle = 0;
tOpenProcess _OpenProcess = 0;
tVirtualAllocEx _VirtualAllocEx = 0;
tVirtualProtectEx _VirtualProtectEx = 0;
tVirtualFreeEx _VirtualFreeEx = 0;
tWriteProcessMemory _WriteProcessMemory = 0;
tReadProcessMemory _ReadProcessMemory = 0;
tCreateRemoteThread _CreateRemoteThread = 0;
tLoadLibraryW _LoadLibraryW = 0;
tIsWow64Process _IsWow64Process = 0;
tGetCurrentProcess _GetCurrentProcess = 0;
tOpenProcessToken _OpenProcessToken = 0;
tLookupPrivilegeValueW _LookupPrivilegeValueW = 0;
tAdjustTokenPrivileges _AdjustTokenPrivileges = 0;

using f_LoadLibraryA = HINSTANCE(WINAPI*)(const char* lpLibFilename);
using f_GetProcAddress = FARPROC(WINAPI*)(HMODULE hModule, LPCSTR lpProcName);
using f_DLL_ENTRY_POINT = BOOL(WINAPI*)(void* hDll, DWORD dwReason, void* pReserved);
#ifdef _WIN64
using f_RtlAddFunctionTable = BOOL(WINAPIV*)(PRUNTIME_FUNCTION FunctionTable, DWORD EntryCount, DWORD64 BaseAddress);
#endif // _WIN64

struct MANUAL_MAPPING_DATA
{
    f_LoadLibraryA pLoadLibraryA;
    f_GetProcAddress pGetProcAddress;
#ifdef _WIN64
    f_RtlAddFunctionTable pRtlAddFunctionTable;
#endif // _WIN64
    BYTE* pbase;
    HINSTANCE hMod;
    DWORD fdwReasonParam;
    LPVOID reservedParam;
    BOOL SEHSupport;
};

typedef enum
{
    FR_READ_WRITE = 0,
    FR_READ_ONLY,
} flag_t;

typedef struct
{
    HANDLE hMap;
    HANDLE handle;
    flag_t access;
    size_t size;
    uint8_t *start;
} File;

File open_or_create_file(const wchar_t *path, flag_t access, int create);
File open_file(const wchar_t *path, flag_t access);
int close_file(HANDLE handle);
int file_exists(const wchar_t *path);
int get_file_size(File *f);
int map_file(File *f);
int unmap_file(File f);

File open_file(const wchar_t *path, flag_t access)
{
    File f = open_or_create_file(path, access, 0);
    if (f.handle == INVALID_HANDLE_VALUE)
    {
        log_error("Couldn't open file: %S", path);
        exit(1);
    }
    return f;
}

File open_or_create_file(const wchar_t *path, flag_t access, int create)
{
    File f = {0};
    DWORD dwCreationDisposition;
    if (create && !file_exists(path))
        dwCreationDisposition = CREATE_NEW;
    else
        dwCreationDisposition = OPEN_EXISTING;

    DWORD dwDesiredAccess;
    switch (access)
    {
        case FR_READ_WRITE:
            dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
            break;
        case FR_READ_ONLY:
            dwDesiredAccess = GENERIC_READ;
            break;
        default:
        {
            assert(0 && "unreachable");
            exit(1);
        }
        break;
    }

    f.handle = CreateFileW(path, dwDesiredAccess, 0, 0, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, 0);
    if (f.handle == INVALID_HANDLE_VALUE)
    {
        log_error("Opening file %S", path);
        return f;
    }
    if (dwCreationDisposition == OPEN_EXISTING && !get_file_size(&f))
    {
        log_error("Invalid file size %S", path);
        close_file(f.handle);
        f.handle = INVALID_HANDLE_VALUE;
        return f;
    }
    f.access = access;
    return f;
}

int close_file(HANDLE handle)
{
    if (_CloseHandle(handle) == 0)
    {
        log_error("CloseHandle failed (%ld)", GetLastError());
        return 0;
    }
    return 1;
}

int file_exists(const wchar_t *path)
{
    DWORD dwAttrib = GetFileAttributesW(path);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
            !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

int get_file_size(File *f)
{
    LARGE_INTEGER lpFileSize;
    if (!GetFileSizeEx(f->handle, &lpFileSize))
    {
        log_error("GetFileSizeEx failed (%ld)", GetLastError());
        return 0;
    }
    f->size = lpFileSize.QuadPart;
    return 1;
}

int map_file(File *f)
{
    DWORD flProtect;
    switch (f->access)
    {
        case FR_READ_WRITE:
            flProtect = PAGE_READWRITE;
            break;
        case FR_READ_ONLY:
            flProtect = PAGE_READONLY;
            break;
        default:
            return 0;
    }

    f->hMap = CreateFileMappingA(f->handle, 0, flProtect, 0, 0, 0);
    if (f->hMap == 0)
    {
        log_error("CreateFileMappingA failed (%ld)", GetLastError());
        _CloseHandle(f->handle);
        return 0;
    }

    DWORD dwDesiredAccess;
    switch (f->access)
    {
        case FR_READ_WRITE:
            dwDesiredAccess = FILE_MAP_ALL_ACCESS;
            break;
        case FR_READ_ONLY:
            dwDesiredAccess = FILE_MAP_READ;
            break;
        default:
            return 0;
    }

    f->start = (uint8_t *)MapViewOfFile(f->hMap, dwDesiredAccess, 0, 0, 0);
    if (f->start == 0)
    {
        log_error("MapViewOfFile failed (%ld)", GetLastError());
        _CloseHandle(f->hMap);
        _CloseHandle(f->handle);
        return 0;
    }
    return 1;
}

int unmap_file(File f)
{
    if (UnmapViewOfFile(f.start) == 0)
    {
        log_error("UnmapViewOfFile failed (%ld)", GetLastError());
        return 0;
    }
    if (_CloseHandle(f.hMap) == 0)
    {
        log_error("_CloseHandle failed (%ld)", GetLastError());
        return 0;
    }
    return 1;
}

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

#pragma runtime_checks( "", off )
#pragma optimize( "", off )
void __stdcall Shellcode(MANUAL_MAPPING_DATA* pData)
{
    if (!pData) {
        pData->hMod = (HINSTANCE)0x404040;
        return;
    }

    BYTE* pBase = pData->pbase;
    auto* pOpt = &reinterpret_cast<IMAGE_NT_HEADERS*>(pBase + reinterpret_cast<IMAGE_DOS_HEADER*>((uintptr_t)pBase)->e_lfanew)->OptionalHeader;

    auto _LoadLibraryA = pData->pLoadLibraryA;
    auto _GetProcAddress = pData->pGetProcAddress;
#ifdef _WIN64
    auto _RtlAddFunctionTable = pData->pRtlAddFunctionTable;
#endif // _WIN64
    auto _DllMain = reinterpret_cast<f_DLL_ENTRY_POINT>(pBase + pOpt->AddressOfEntryPoint);

    BYTE* LocationDelta = pBase - pOpt->ImageBase;
    if (LocationDelta) {
        if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
            auto* pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION*>(pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
            const auto* pRelocEnd = reinterpret_cast<IMAGE_BASE_RELOCATION*>(reinterpret_cast<uintptr_t>(pRelocData) + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size);
            while (pRelocData < pRelocEnd && pRelocData->SizeOfBlock) {
                UINT AmountOfEntries = (pRelocData->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
                WORD* pRelativeInfo = reinterpret_cast<WORD*>(pRelocData + 1);

                for (UINT i = 0; i != AmountOfEntries; ++i, ++pRelativeInfo) {
                    if (RELOC_FLAG(*pRelativeInfo)) {
                        UINT_PTR* pPatch = reinterpret_cast<UINT_PTR*>(pBase + pRelocData->VirtualAddress + ((*pRelativeInfo) & 0xFFF));
                        *pPatch += reinterpret_cast<UINT_PTR>(LocationDelta);
                    }
                }
                pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION*>(reinterpret_cast<BYTE*>(pRelocData) + pRelocData->SizeOfBlock);
            }
        }
    }

    if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
        auto* pImportDescr = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
        while (pImportDescr->Name) {
            char* szMod = reinterpret_cast<char*>(pBase + pImportDescr->Name);
            HINSTANCE hDll = _LoadLibraryA(szMod);

            ULONG_PTR* pThunkRef = reinterpret_cast<ULONG_PTR*>(pBase + pImportDescr->OriginalFirstThunk);
            ULONG_PTR* pFuncRef = reinterpret_cast<ULONG_PTR*>(pBase + pImportDescr->FirstThunk);

            if (!pThunkRef)
                pThunkRef = pFuncRef;

            for (; *pThunkRef; ++pThunkRef, ++pFuncRef) {
                if (IMAGE_SNAP_BY_ORDINAL(*pThunkRef)) {
                    *pFuncRef = (ULONG_PTR)_GetProcAddress(hDll, reinterpret_cast<char*>(*pThunkRef & 0xFFFF));
                }
                else {
                    auto* pImport = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(pBase + (*pThunkRef));
                    *pFuncRef = (ULONG_PTR)_GetProcAddress(hDll, pImport->Name);
                }
            }
            ++pImportDescr;
        }
    }

    if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size) {
        auto* pTLS = reinterpret_cast<IMAGE_TLS_DIRECTORY*>(pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
        auto* pCallback = reinterpret_cast<PIMAGE_TLS_CALLBACK*>(pTLS->AddressOfCallBacks);
        for (; pCallback && *pCallback; ++pCallback)
            (*pCallback)(pBase, DLL_PROCESS_ATTACH, NULL);
    }

    bool ExceptionSupportFailed = false;

#ifdef _WIN64
    if (pData->SEHSupport) {
        auto excep = pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
        if (excep.Size) {
            if (!_RtlAddFunctionTable(
                reinterpret_cast<IMAGE_RUNTIME_FUNCTION_ENTRY*>(pBase + excep.VirtualAddress),
                excep.Size / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY), (DWORD64)pBase)) {
                ExceptionSupportFailed = true;
            }
        }
    }
#endif // _WIN64

    _DllMain(pBase, pData->fdwReasonParam, pData->reservedParam);

    if (ExceptionSupportFailed)
        pData->hMod = reinterpret_cast<HINSTANCE>(0x505050);
    else
        pData->hMod = reinterpret_cast<HINSTANCE>(pBase);
}

//Note: Exception support only x64 with build params /EHa or /EHc
bool manual_map_dll(HANDLE hProc, BYTE* pSrcData, bool ClearHeader = true, bool ClearNonNeededSections = true, bool AdjustProtections = true,
                  bool SEHExceptionSupport = true, DWORD fdwReason = DLL_PROCESS_ATTACH, LPVOID lpReserved = 0);

bool manual_map_dll(HANDLE hProc, BYTE* pSrcData, bool ClearHeader, bool ClearNonNeededSections, bool AdjustProtections,
                    bool SEHExceptionSupport, DWORD fdwReason, LPVOID lpReserved)
{
    IMAGE_NT_HEADERS* pOldNtHeader = NULL;
    IMAGE_OPTIONAL_HEADER* pOldOptHeader = NULL;
    IMAGE_FILE_HEADER* pOldFileHeader = NULL;
    BYTE* pTargetBase = NULL;

    if (reinterpret_cast<IMAGE_DOS_HEADER*>(pSrcData)->e_magic != 0x5A4D) {
        log_error("Invalid file format");
        return false;
    }

    pOldNtHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(pSrcData + reinterpret_cast<IMAGE_DOS_HEADER*>(pSrcData)->e_lfanew);
    pOldOptHeader = &pOldNtHeader->OptionalHeader;
    pOldFileHeader = &pOldNtHeader->FileHeader;

    if (pOldFileHeader->Machine != CURRENT_ARCH) {
        log_error("Invalid platform");
        return false;
    }

    pTargetBase = reinterpret_cast<BYTE*>(_VirtualAllocEx(hProc, NULL, pOldOptHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    if (!pTargetBase) {
        log_error("Target process memory allocation failed 0x%X", _GetLastError());
        return false;
    }

    DWORD oldp = 0;
    _VirtualProtectEx(hProc, pTargetBase, pOldOptHeader->SizeOfImage, PAGE_EXECUTE_READWRITE, &oldp);

    MANUAL_MAPPING_DATA data {0};
    data.pLoadLibraryA = LoadLibraryA;
    data.pGetProcAddress = GetProcAddress;
#ifdef _WIN64
    data.pRtlAddFunctionTable = (f_RtlAddFunctionTable)RtlAddFunctionTable;
#else 
    SEHExceptionSupport = false;
#endif // _WIN64
    data.pbase = pTargetBase;
    data.fdwReasonParam = fdwReason;
    data.reservedParam = lpReserved;
    data.SEHSupport = SEHExceptionSupport;

    if (!_WriteProcessMemory(hProc, pTargetBase, pSrcData, 0x1000, NULL)) {
        log_error("Couldn't write file header 0x%X", _GetLastError());
        _VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
        return false;
    }

    IMAGE_SECTION_HEADER* pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
    for (UINT i = 0; i != pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
        if (pSectionHeader->SizeOfRawData) {
            if (!_WriteProcessMemory(hProc, pTargetBase + pSectionHeader->VirtualAddress, pSrcData + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData, NULL)) {
                log_error("Couldn't map sections: 0x%X", _GetLastError());
                _VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
                return false;
            }
        }
    }

    BYTE* MappingDataAlloc = reinterpret_cast<BYTE*>(_VirtualAllocEx(hProc, NULL, sizeof(MANUAL_MAPPING_DATA), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    if (!MappingDataAlloc) {
        log_error("Target process mapping allocation failed 0x%X", _GetLastError());
        _VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
        return false;
    }

    if (!_WriteProcessMemory(hProc, MappingDataAlloc, &data, sizeof(MANUAL_MAPPING_DATA), NULL)) {
        log_error("Couldn't write mapping 0x%X", _GetLastError());
        _VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
        _VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE);
        return false;
    }

    void* pShellcode = _VirtualAllocEx(hProc, NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!pShellcode) {
        log_error("Memory shellcode allocation failed 0x%X", _GetLastError());
        _VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
        _VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE);
        return false;
    }

    if (!_WriteProcessMemory(hProc, pShellcode, Shellcode, 0x1000, NULL)) {
        log_error("Couldn't write shellcode 0x%X", _GetLastError());
        _VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
        _VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE);
        _VirtualFreeEx(hProc, pShellcode, 0, MEM_RELEASE);
        return false;
    }

    log_info("Mapped at %p", pTargetBase);
    log_info("Mapping info at %p", MappingDataAlloc);
    log_info("Shellcode at %p", pShellcode);

    HANDLE hThread = _CreateRemoteThread(hProc, NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(pShellcode), MappingDataAlloc, 0, NULL);
    if (!hThread) {
        log_error("Thread creation failed 0x%X", _GetLastError());
        _VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
        _VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE);
        _VirtualFreeEx(hProc, pShellcode, 0, MEM_RELEASE);
        return false;
    }
    _CloseHandle(hThread);

    log_info("Thread created at: %p, waiting for return.", pShellcode);

    HINSTANCE hCheck = NULL;
    while (!hCheck) {
        DWORD exitcode = 0;
        GetExitCodeProcess(hProc, &exitcode);
        if (exitcode != STILL_ACTIVE) {
            log_error("Process crashed, exit code: %d", exitcode);
            return false;
        }

        MANUAL_MAPPING_DATA data_checked {0};
        _ReadProcessMemory(hProc, MappingDataAlloc, &data_checked, sizeof(data_checked), NULL);
        hCheck = data_checked.hMod;

        if (hCheck == (HINSTANCE)0x404040) {
            log_error("Wrong mapping ptr");
            _VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
            _VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE);
            _VirtualFreeEx(hProc, pShellcode, 0, MEM_RELEASE);
            return false;
        }
        else if (hCheck == (HINSTANCE)0x505050) {
            log_warn("Exception support failed");
        }
        Sleep(10);
    }

    BYTE* emptyBuffer = (BYTE*)malloc(1024 * 1024 * 20);
    if (emptyBuffer == NULL) {
        log_error("Unable to allocate memory");
        return false;
    }
    SecureZeroMemory(emptyBuffer, 1024 * 1024 * 20);

    if (ClearHeader) {
        if (!_WriteProcessMemory(hProc, pTargetBase, emptyBuffer, 0x1000, NULL)) {
            log_warn("Couldn't clear header");
        }
    }

    if (ClearNonNeededSections) {
        auto pdata_s = mks(".pdata");
        auto rsrc_s = mks(".rsrc");
        auto reloc_s = mks(".reloc");
        pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
        for (UINT i = 0; i != pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
            if (pSectionHeader->Misc.VirtualSize) {
                if ((SEHExceptionSupport ? 0 : strcmp((char*)pSectionHeader->Name, pdata_s.c_str()) == 0) ||
                    strcmp((char*)pSectionHeader->Name, rsrc_s.c_str()) == 0 ||
                    strcmp((char*)pSectionHeader->Name, reloc_s.c_str()) == 0) {
                    log_info("Clearing %s", pSectionHeader->Name);
                    if (!_WriteProcessMemory(hProc, pTargetBase + pSectionHeader->VirtualAddress, emptyBuffer, pSectionHeader->Misc.VirtualSize, NULL)) {
                        log_error("Couldn't clear section %s: 0x%X", pSectionHeader->Name, _GetLastError());
                    }
                }
            }
        }
    }

    if (AdjustProtections) {
        pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
        for (UINT i = 0; i != pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
            if (pSectionHeader->Misc.VirtualSize) {
                DWORD old = 0;
                DWORD newP = PAGE_READONLY;

                if ((pSectionHeader->Characteristics & IMAGE_SCN_MEM_WRITE) > 0) {
                    newP = PAGE_READWRITE;
                }
                else if ((pSectionHeader->Characteristics & IMAGE_SCN_MEM_EXECUTE) > 0) {
                    newP = PAGE_EXECUTE_READ;
                }
                if (_VirtualProtectEx(hProc, pTargetBase + pSectionHeader->VirtualAddress, pSectionHeader->Misc.VirtualSize, newP, &old)) {
                    log_info("Section %s set as %lX", (char*)pSectionHeader->Name, newP);
                }
                else {
                    log_error("Section %s not set as %lX", (char*)pSectionHeader->Name, newP);
                }
            }
        }
        DWORD old = 0;
        _VirtualProtectEx(hProc, pTargetBase, IMAGE_FIRST_SECTION(pOldNtHeader)->VirtualAddress, PAGE_READONLY, &old);
    }

    if (!_WriteProcessMemory(hProc, pShellcode, emptyBuffer, 0x1000, NULL)) {
        log_warn("Couldn't clear shellcode");
    }
    if (!_VirtualFreeEx(hProc, pShellcode, 0, MEM_RELEASE)) {
        log_warn("Couldn't release shell code memory");
    }
    if (!_VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE)) {
        log_warn("Couldn't release mapping data memory");
    }

    return true;
}

static inline bool is_correct_target_arch(HANDLE hProc)
{
    BOOL bTarget = FALSE;
    if (!_IsWow64Process(hProc, &bTarget))
        return false;
    BOOL bHost = FALSE;
    _IsWow64Process(_GetCurrentProcess(), &bHost);
    return (bTarget == bHost);
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
    auto sAdvapi32Dll(mks("Advapi32.dll"));
    auto k32 = GetModuleHandleA(sKernel32Dll.c_str());
    auto a32 = LoadLibraryA(sAdvapi32Dll.c_str());
    k32func(CreateToolhelp32Snapshot);
    k32func(GetFullPathNameW);
    k32func(GetLastError);
    k32func(Process32FirstW);
    k32func(Process32NextW);
    k32func(CloseHandle);
    k32func(OpenProcess);
    k32func(VirtualAllocEx);
    k32func(VirtualProtectEx);
    k32func(VirtualFreeEx);
    k32func(WriteProcessMemory);
    k32func(ReadProcessMemory);
    k32func(CreateRemoteThread);
    k32func(LoadLibraryW);
    k32func(IsWow64Process);
    k32func(GetCurrentProcess);
    a32func(OpenProcessToken);
    a32func(LookupPrivilegeValueW);
    a32func(AdjustTokenPrivileges);

    auto process_name_s = mks("osu!.exe");
    auto process_name_w = std::wstring(process_name_s.begin(), process_name_s.end());
    auto dll_name_s = mks("freedom.dll");
    auto dll_name_w = std::wstring(dll_name_s.begin(), dll_name_s.end());

    const wchar_t *process_name = argc > 1 ? argv[1] : process_name_w.c_str();
    DWORD process_id = get_process_id(process_name);
    if (process_id == 0)
    {
        log_error("Failed to get process id: launch %S first!", process_name);
        return 1;
    }

    TOKEN_PRIVILEGES priv = {0};
    HANDLE hToken = NULL;
    if (_OpenProcessToken(_GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        priv.PrivilegeCount = 1;
        priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        if (_LookupPrivilegeValueW(NULL, SE_DEBUG_NAME, &priv.Privileges[0].Luid))
            _AdjustTokenPrivileges(hToken, FALSE, &priv, 0, NULL, NULL);

        _CloseHandle(hToken);
    }

    HANDLE hProc = _OpenProcess(PROCESS_ALL_ACCESS, 0, process_id);
    if (hProc == NULL)
    {
        log_error("Couldn't open process: 0x%X", _GetLastError());
        return 1;
    }

    if (!is_correct_target_arch(hProc))
    {
        log_error("Couldn't confirm target process architecture: 0x%X", _GetLastError());
        _CloseHandle(hProc);
        return 1;
    }

    const wchar_t *dll_name = argc > 2 ? argv[2] : dll_name_w.c_str();
    static wchar_t module_path[MAX_PATH * 2];
    DWORD module_path_length = _GetFullPathNameW(dll_name, MAX_PATH * 2, module_path, NULL);
    if (module_path_length == 0)
    {
        log_error("Failed to retrieve the full path and file name of the dll. (0x%X)", _GetLastError());
        _CloseHandle(hProc);
        return 1;
    }

    if (!file_exists(module_path))
    {
        log_error("File %S doesn't exist", module_path);
        _CloseHandle(hProc);
        return 1;
    }

    File module_file = open_file(module_path, FR_READ_ONLY);
    if (!map_file(&module_file))
    {
        log_error("Couldn't map module file");
        close_file(module_file.handle);
        _CloseHandle(hProc);
        return 1;
    }

    LPVOID config_path = _VirtualAllocEx(hProc, NULL, sizeof(module_path), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!config_path)
        log_warn("Couldn't allocate config path 0x%X", _GetLastError());
    else if (!_WriteProcessMemory(hProc, config_path, module_path, sizeof(module_path), NULL))
        log_warn("Couldn't write config path 0x%X", _GetLastError());

    manual_map_dll(hProc, (BYTE *)module_file.start, true, true, true, true, DLL_PROCESS_ATTACH, config_path);

    unmap_file(module_file);
    close_file(module_file.handle);

    _CloseHandle(hProc);
    return 0;
}
