// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "log.h"

ImGuiLogger debug_log;

ImGuiLogger::ImGuiLogger() {}

ImGuiLogger::ImGuiLogger(const ImVec2 &size) : size(size) {}

void ImGuiLogger::resize(const ImVec2 &new_size)
{
    size = new_size;
}

void ImGuiLogger::clear()
{
    buf.clear();
}

void ImGuiLogger::add(const char *fmt, ...)
{
    if (buf.size() >= 1 << 12)
        buf.clear();
    va_list args;
    va_start(args, fmt);
    buf.appendfv(fmt, args);
    va_end(args);
    ScrollToBottom = true;
}

void ImGuiLogger::draw()
{
    ImGui::TextWrapped("%s", buf.begin());
    if (ScrollToBottom)
    {
        ImGui::SetScrollHereY(1.0f);
        ScrollToBottom = false;
    }
}
