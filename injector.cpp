#include <windows.h>
#include <tlhelp32.h>

#include <stdio.h>
#include <string.h>

DWORD get_process_id(const wchar_t *process_name)
{
    DWORD process_id = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);

        if (Process32First(hSnap, &procEntry))
        {
            do
            {
                if (!_wcsicmp(procEntry.szExeFile, process_name))
                {
                    process_id = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &procEntry));
        }
    }
    CloseHandle(hSnap);
    return process_id;
}

int wmain(int argc, wchar_t **argv, wchar_t **envp)
{
    wchar_t *process_name = argc > 1 ? argv[1] : L"osu!.exe";
    DWORD process_id = get_process_id(process_name);
    if (process_id == 0)
    {
        fprintf(stderr, "get_process_id failed: launch %S first!\n", process_name);
        return 1;
    }

    wchar_t *dll_name = argc > 2 ? argv[2] : L"freedom.dll";
    static wchar_t module_path[MAX_PATH * 2];
    DWORD module_path_length = GetFullPathNameW(dll_name, MAX_PATH * 2, module_path, NULL);
    if (module_path_length == 0)
    {
        fprintf(stderr, "GetFullPathNameA failed: %ld\n", GetLastError());
        return 1;
    }

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, process_id);
    if (hProc != NULL)
    {
        void *loc = VirtualAllocEx(hProc, 0, module_path_length * sizeof(wchar_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (loc)
        {
            if (WriteProcessMemory(hProc, loc, module_path, module_path_length * sizeof(wchar_t), 0))
            {
                HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryW, loc, 0, 0);
                if (hThread)
                    CloseHandle(hThread);
                else
                    fprintf(stderr, "CreateRemoteThread failed: %ld\n", GetLastError());
            }
            else
                fprintf(stderr, "WriteProcessMemory failed: %ld\n", GetLastError());
        }
        else
            fprintf(stderr, "VirtualAllocEx failed: %ld\n", GetLastError());
    }
    else
        fprintf(stderr, "OpenProcess failed: %ld\n", GetLastError());

    if (hProc)
        CloseHandle(hProc);

    return 0;
}
