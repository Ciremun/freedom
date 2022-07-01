#include "config.h"

bool ar_lock = true;
float ar_value = 10.0f;
bool mod_menu_visible = true;

const char *get_imgui_ini_filename(HMODULE hMod)
{
    static wchar_t module_path[MAX_PATH * 2];
    DWORD module_path_length = GetModuleFileNameW(hMod, module_path, MAX_PATH * 2);
    if (module_path_length == 0)
        return 0;

    static char module_path_u8[MAX_PATH * 2];
    int module_path_u8_length = WideCharToMultiByte(CP_UTF8, 0, module_path, module_path_length, module_path_u8, MAX_PATH * 2, 0, 0);
    if (module_path_u8_length == 0)
        return 0;

    module_path_u8[module_path_u8_length] = '\0';

    DWORD backslash_index = module_path_u8_length - 1;
    while (backslash_index)
        if (module_path_u8[--backslash_index] == '\\')
            break;

    memcpy(module_path_u8 + backslash_index + 1, "freedom.ini", 12);

    return (const char *)&module_path_u8;
}

static void FreedomHandler_ClearAll(ImGuiContext *ctx, ImGuiSettingsHandler *) {}
static void FreedomHandler_ApplyAll(ImGuiContext *ctx, ImGuiSettingsHandler *) {}
static void *FreedomHandler_ReadOpen(ImGuiContext *, ImGuiSettingsHandler *, const char *name) { return (void *)1; }

static void FreedomHandler_WriteAll(ImGuiContext *ctx, ImGuiSettingsHandler *handler, ImGuiTextBuffer *buf)
{
    buf->reserve(buf->size() + (1 + 4) * 2);
    buf->appendf("[%s][%s]\n", handler->TypeName, "Settings");
    buf->appendf("ar_lock=%d\n", (int)ar_lock);
    buf->appendf("ar_value=%.1f\n", ar_value);
    buf->appendf("visible=%d\n", mod_menu_visible);
    buf->append("\n");
}

static void FreedomHandler_ReadLine(ImGuiContext *, ImGuiSettingsHandler *, void *, const char *line)
{
    int ar_lock_i, mod_menu_visible_i;
    float ar_value_f;
    if (sscanf(line, "ar_lock=%d", &ar_lock_i) == 1)
        ar_lock = ar_lock_i;
    else if (sscanf(line, "ar_value=%f", &ar_value_f) == 1)
        ar_value = ar_value_f;
    else if (sscanf(line, "visible=%d", &mod_menu_visible_i) == 1)
        mod_menu_visible = mod_menu_visible_i;
}

void set_imgui_ini_handler()
{
    ImGuiSettingsHandler ini_handler;
    ini_handler.TypeName = "Freedom";
    ini_handler.TypeHash = ImHashStr("Freedom");
    ini_handler.ClearAllFn = FreedomHandler_ClearAll;
    ini_handler.ReadOpenFn = FreedomHandler_ReadOpen;
    ini_handler.ReadLineFn = FreedomHandler_ReadLine;
    ini_handler.ApplyAllFn = FreedomHandler_ApplyAll;
    ini_handler.WriteAllFn = FreedomHandler_WriteAll;
    ImGui::AddSettingsHandler(&ini_handler);
}
