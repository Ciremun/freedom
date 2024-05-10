#pragma once

#include <vector>

#include "imgui.h"

#include "ui/colors.h"

#ifdef FR_LOG_TO_CONSOLE
    #define FR_INFO(fmt, ...) fprintf(stdout, fmt "\n", __VA_ARGS__)
    #ifdef NDEBUG
        #define FR_ERROR(fmt, ...) fprintf(stderr, "[!] " fmt "\n", __VA_ARGS__)
    #else
        #define FR_ERROR(fmt, ...) fprintf(stderr, "[!] (%s) %s:%d: " fmt "\n", __FILE__, __FUNCSIG__, __LINE__, __VA_ARGS__)
    #endif // NDEBUG
#else
    #define FR_INFO(fmt, ...) debug_log.add(fmt, __VA_ARGS__)
    #ifdef NDEBUG
        #define FR_ERROR(fmt, ...) debug_log.add("[!] " fmt, __VA_ARGS__)
    #else
        #define FR_ERROR(fmt, ...) debug_log.add("[!] (%s) %s:%d: " fmt, __FILE__, __FUNCSIG__, __LINE__, __VA_ARGS__)
    #endif // NDEBUG
#endif // FR_LOG_TO_CONSOLE

#define FR_PTR_INFO(...) FR_INFO("%-35.35s 0x%X", __VA_ARGS__)

struct ImGuiLogger
{
    std::vector<ImGuiTextBuffer *> lines;
    bool ScrollToBottom = true;
    ImVec2 size = ImVec2(.0f, .0f);

    ImGuiLogger();
    ImGuiLogger(const ImVec2 &size);

    void clear();
    void resize(const ImVec2 &new_size);
    void add(const char *fmt, ...);
    void draw();
};

extern ImGuiLogger debug_log;
