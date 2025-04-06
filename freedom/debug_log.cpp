#include "ui/debug_log.h"

ImGuiLogger debug_log;

ImGuiLogger::ImGuiLogger() { lines.reserve(128); }
ImGuiLogger::ImGuiLogger(const ImVec2 &size) : size(size) { lines.reserve(128); }

void ImGuiLogger::resize(const ImVec2 &new_size)
{
    size = new_size;
}

void ImGuiLogger::clear()
{
    for (const auto &line : lines)
    {
        line->clear();
        delete line;
    }
    lines.clear();
}

void ImGuiLogger::add(const char *fmt, ...)
{
    extern bool cfg_show_debug_log;
    if (!cfg_show_debug_log)
        return;
    if (lines.size() >= 1024)
        clear();
    va_list args;
    va_start(args, fmt);
    ImGuiTextBuffer *line = new ImGuiTextBuffer();
    line->appendfv(fmt, args);
    lines.push_back(line);
    va_end(args);
    ScrollToBottom = true;
}

static inline void TextColored(const char *line_begin, ImVec4 color)
{
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextWrapped("%s", line_begin);
    ImGui::PopStyleColor();
}

void ImGuiLogger::draw()
{
    ImGui::BeginChild("##debug_log", ImVec2(.0f, -30.f));
    for (const auto &line : lines)
    {
        if (line->size() >= 3 && line->c_str()[0] == '[' && line->Buf.Data[2] == ']')
        {
            if (line->Buf.Data[1] == '+') TextColored(line->begin(), LOG_OK);
            if (line->Buf.Data[1] == '!') TextColored(line->begin(), LOG_ERROR);
        }
        else
            ImGui::TextWrapped("%s", line->begin());
    }
    if (ScrollToBottom)
    {
        ImGui::SetScrollHereY(1.0f);
        ScrollToBottom = false;
    }
    ImGui::EndChild();
}
