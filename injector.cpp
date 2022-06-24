#ifdef UNICODE
#undef UNICODE
#endif // UNICODE

#include <windows.h>
#include <tlhelp32.h>
#include <string.h>
#include <stdio.h>

DWORD GetProcID(const char* process_name)
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
                if (!_stricmp(procEntry.szExeFile, process_name))
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

int main(int argc, char** argv)
{
    char *process_name;
    if (argc > 1)
        process_name = argv[1];
    else
        process_name = "osu!.exe";

    DWORD process_id = 0;
    while ((process_id = GetProcID(process_name)) == 0)
    {
        printf("Info: waiting for %s\r", process_name);
        fflush(stdout);
        Sleep(100);
    }

    CHAR module_path[MAX_PATH] = {0};
    if (GetFullPathNameA("freedom.dll", MAX_PATH, module_path, NULL) == 0)
    {
        fprintf(stderr, "GetFullPathNameA failed: %ld\n", GetLastError());
        return 1;
    }

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, process_id);

    if (hProc != INVALID_HANDLE_VALUE)
    {
        void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (loc)
        {
            if (WriteProcessMemory(hProc, loc, module_path, strlen(module_path) + 1, 0) != 0)
            {
                HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);
                if (hThread)
                    CloseHandle(hThread);
            }
        }

    }

    if (hProc)
        CloseHandle(hProc);

    return 0;
}