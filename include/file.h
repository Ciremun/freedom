#pragma once

#include <windows.h>

#include <stdint.h>

struct File
{
    HANDLE hMap;
    HANDLE handle;
    size_t size;
    uint8_t *start;
};

File open_file(const wchar_t *path);
int close_file(HANDLE handle);
int map_file(File *f);
int unmap_file(File f);
int unmap_and_close_file(File f);
int get_file_size(File *f);
