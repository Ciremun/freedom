#include "config.h"

int cfg_font_size = 30;
int cfg_spins_per_minute = 300;
bool cfg_mod_menu_visible = true;
float cfg_fraction_modifier = 0.04f;
bool cfg_replay_enabled = false;
bool cfg_replay_aim = true;
bool cfg_replay_keys = true;

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

    FR_INFO_FMT("freedom.ini path: %s", module_path_u8);

    return (const char *)&module_path_u8;
}

static void FreedomHandler_ClearAll(ImGuiContext *ctx, ImGuiSettingsHandler *) {}
static void FreedomHandler_ApplyAll(ImGuiContext *ctx, ImGuiSettingsHandler *) {}
static void *FreedomHandler_ReadOpen(ImGuiContext *, ImGuiSettingsHandler *, const char *name) { return (void *)1; }

static void FreedomHandler_WriteAll(ImGuiContext *ctx, ImGuiSettingsHandler *handler, ImGuiTextBuffer *buf)
{
    buf->reserve(buf->size() + (1 + 4) * 2);
    buf->appendf("[%s][%s]\n", handler->TypeName, "Settings");
    buf->appendf("ar_lock=%d\n", (int)ar_parameter.lock);
    buf->appendf("ar_value=%.1f\n", ar_parameter.value);
    buf->appendf("cs_lock=%d\n", (int)cs_parameter.lock);
    buf->appendf("cs_value=%.1f\n", cs_parameter.value);
    buf->appendf("od_lock=%d\n", (int)od_parameter.lock);
    buf->appendf("od_value=%.1f\n", od_parameter.value);
    buf->appendf("visible=%d\n", cfg_mod_menu_visible);
    buf->appendf("font_size=%d\n", cfg_font_size);
    buf->appendf("relax=%d\n", cfg_relax_lock);
    buf->appendf("aimbot=%d\n", cfg_aimbot_lock);
    buf->appendf("spins_per_minute=%d\n", cfg_spins_per_minute);
    buf->appendf("fraction_modifier=%.3f\n", cfg_fraction_modifier);
    buf->appendf("replay=%d\n", (int)cfg_replay_enabled);
    buf->appendf("replay_aim=%d\n", (int)cfg_replay_aim);
    buf->appendf("replay_keys=%d\n", (int)cfg_replay_keys);
    buf->append("\n");
}

static void FreedomHandler_ReadLine(ImGuiContext *, ImGuiSettingsHandler *, void *, const char *line)
{
    int ar_lock_i, cs_lock_i, od_lock_i, mod_menu_visible_i, font_size_i, relax_lock_i, aimbot_lock_i, spins_per_minute_i;
    int replay_i, replay_aim_i, replay_keys_i;
    float ar_value_f, cs_value_f, od_value_f, fraction_modifier_f;
    if (sscanf(line, "ar_lock=%d", &ar_lock_i) == 1)
        ar_parameter.lock = ar_lock_i;
    else if (sscanf(line, "ar_value=%f", &ar_value_f) == 1)
        ar_parameter.value = ar_value_f;
    else if (sscanf(line, "cs_lock=%d", &cs_lock_i) == 1)
        cs_parameter.lock = cs_lock_i;
    else if (sscanf(line, "cs_value=%f", &cs_value_f) == 1)
        cs_parameter.value = cs_value_f;
    else if (sscanf(line, "od_lock=%d", &od_lock_i) == 1)
        od_parameter.lock = od_lock_i;
    else if (sscanf(line, "od_value=%f", &od_value_f) == 1)
        od_parameter.value = od_value_f;
    else if (sscanf(line, "visible=%d", &mod_menu_visible_i) == 1)
        cfg_mod_menu_visible = mod_menu_visible_i;
    else if (sscanf(line, "font_size=%d", &font_size_i) == 1)
        cfg_font_size = font_size_i;
    else if (sscanf(line, "relax=%d", &relax_lock_i) == 1)
        cfg_relax_lock = relax_lock_i;
    else if (sscanf(line, "aimbot=%d", &aimbot_lock_i) == 1)
        cfg_aimbot_lock = aimbot_lock_i;
    else if (sscanf(line, "spins_per_minute=%d", &spins_per_minute_i) == 1)
        cfg_spins_per_minute = spins_per_minute_i;
    else if (sscanf(line, "fraction_modifier=%f", &fraction_modifier_f) == 1)
        cfg_fraction_modifier = fraction_modifier_f;
    else if (sscanf(line, "replay=%d", &replay_i) == 1)
        cfg_replay_enabled = replay_i;
    else if (sscanf(line, "replay_aim=%d", &replay_aim_i) == 1)
        cfg_replay_aim = replay_aim_i;
    else if (sscanf(line, "replay_keys=%d", &replay_keys_i) == 1)
        cfg_replay_keys = replay_keys_i;
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
