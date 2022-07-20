#include "file.h"

File open_file(const wchar_t *path)
{
    File f = {0};
    f.handle = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    return f;
}

int close_file(HANDLE handle)
{
    if (CloseHandle(handle) == 0)
        return 0;
    return 1;
}

int map_file(File *f)
{
    f->hMap = CreateFileMappingA(f->handle, 0, PAGE_READONLY, 0, 0, 0);
    if (f->hMap == 0)
    {
        CloseHandle(f->handle);
        return 0;
    }
    f->start = (uint8_t *)MapViewOfFile(f->hMap, FILE_MAP_READ, 0, 0, 0);
    if (f->start == 0)
    {
        CloseHandle(f->hMap);
        CloseHandle(f->handle);
        return 0;
    }
    return 1;
}

int unmap_file(File f)
{
    if (UnmapViewOfFile(f.start) == 0)
        return 0;
    if (CloseHandle(f.hMap) == 0)
        return 0;
    return 1;
}

int unmap_and_close_file(File f)
{
    return unmap_file(f) && close_file(f.handle);
}

int get_file_size(File *f)
{
    LARGE_INTEGER lpFileSize;
    if (!GetFileSizeEx(f->handle, &lpFileSize))
        return 0;
    f->size = lpFileSize.QuadPart;
    return 1;
}
