#pragma once

#include <vector>

#include "imgui.h"

#include "colors.h"

#ifdef FR_LOG_TO_CONSOLE
#define FR_ERROR(message) fprintf(stderr, "ERROR: %s:%d: %s\n", __FUNCSIG__, __LINE__, message)
#define FR_INFO(message) fprintf(stdout, "%s\n", message)
#define FR_INFO_FMT(fmt, ...) fprintf(stdout, fmt "\n", __VA_ARGS__)
#else
#define FR_ERROR(message) debug_log.add("ERROR: %s:%d: %s\n", __FUNCSIG__, __LINE__, message)
#define FR_INFO(message) debug_log.add("%s\n", message)
#define FR_INFO_FMT(fmt, ...) debug_log.add(fmt "\n", __VA_ARGS__)
#endif // NDEBUG

#define FR_PTR_INFO(...) FR_INFO_FMT("%-35.35s 0x%X", __VA_ARGS__)

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
