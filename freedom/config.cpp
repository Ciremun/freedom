#include "config.h"

int cfg_font_size = 30;
int cfg_spins_per_minute = 300;
bool cfg_mod_menu_visible = true;
float cfg_fraction_modifier = 100.f;
bool cfg_replay_enabled = false;
bool cfg_replay_aim = true;
bool cfg_replay_keys = true;
bool cfg_replay_hardrock = false;
int cfg_relax_style = 'a'; // alternate
bool cfg_score_multiplier_enabled = false;
float cfg_score_multiplier_value = 1.f;
bool cfg_discord_rich_presence_enabled = false;
bool cfg_flashlight_enabled = false;
bool cfg_timewarp_enabled = false;
double cfg_timewarp_playback_rate = 200.0;
bool cfg_relax_checks_od = true;
bool cfg_jumping_window = true;
bool cfg_relax_lock = false;
bool cfg_aimbot_lock = false;
bool cfg_hidden_remover_enabled = false;

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

    memcpy(module_path_u8 + backslash_index + 1, "config.ini", sizeof("config.ini"));

    FR_INFO_FMT("config.ini path: %s", module_path_u8);

    return (const char *)&module_path_u8;
}

static void ConfigHandler_ClearAll(ImGuiContext *ctx, ImGuiSettingsHandler *) {}
static void ConfigHandler_ApplyAll(ImGuiContext *ctx, ImGuiSettingsHandler *) {}
static void *ConfigHandler_ReadOpen(ImGuiContext *, ImGuiSettingsHandler *, const char *name) { return (void *)1; }

static void ConfigHandler_WriteAll(ImGuiContext *ctx, ImGuiSettingsHandler *handler, ImGuiTextBuffer *buf)
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
    buf->appendf("relax_style=%c\n", (char)cfg_relax_style);
    buf->appendf("relax_checks_od=%d\n", (int)cfg_relax_checks_od);
    buf->appendf("aimbot=%d\n", cfg_aimbot_lock);
    buf->appendf("spins_per_minute=%d\n", cfg_spins_per_minute);
    buf->appendf("fraction_modifier=%.3f\n", cfg_fraction_modifier);
    buf->appendf("replay=%d\n", (int)cfg_replay_enabled);
    buf->appendf("replay_aim=%d\n", (int)cfg_replay_aim);
    buf->appendf("replay_keys=%d\n", (int)cfg_replay_keys);
    buf->appendf("sm_lock=%d\n", (int)cfg_score_multiplier_enabled);
    buf->appendf("sm_value=%.2f\n", cfg_score_multiplier_value);
    buf->appendf("drpc=%d\n", (int)cfg_discord_rich_presence_enabled);
    buf->appendf("fl=%d\n", (int)cfg_flashlight_enabled);
    buf->appendf("hd=%d\n", (int)cfg_hidden_remover_enabled);
    buf->appendf("tw_lock=%d\n", (int)cfg_timewarp_enabled);
    buf->appendf("tw_value=%.1lf\n", cfg_timewarp_playback_rate);
    buf->appendf("jump_window=%d\n", (int)cfg_jumping_window);
    buf->append("; PREJIT STUFF\n");
    buf->appendf("CSLoad=%s\n", cm_load_s.c_str());
    buf->appendf("CSReplay=%s\n", cm_replay_s.c_str());
    buf->appendf("CSScore=%s\n", cm_score_s.c_str());
    buf->appendf("CSCheckFlashlight=%s\n", cm_checkflashlight_s.c_str());
    buf->appendf("CSUpdateFlashlight=%s\n", cm_updateflashlight_s.c_str());
    buf->appendf("CSCheckTime=%s\n", cm_checktime_s.c_str());
    buf->appendf("CSUpdateVariables=%s\n", cm_updatevariables_s.c_str());
    buf->append("\n");
}

static void ConfigHandler_ReadLine(ImGuiContext *, ImGuiSettingsHandler *, void *, const char *line)
{
    int ar_lock_i, cs_lock_i, od_lock_i, mod_menu_visible_i, font_size_i,
        relax_lock_i, aimbot_lock_i, spins_per_minute_i, discord_rich_presence_enabled_i,
        hidden_remover_enabled_i, flashlight_enabled_i, timewarp_enabled_i, relax_checks_od_i, jump_window_i;
    int replay_i, replay_aim_i, replay_keys_i, score_multiplier_i;
    float ar_value_f, cs_value_f, od_value_f, fraction_modifier_f, score_multiplier_value_f;
    double timewarp_playback_rate_d;
    char relax_style_c;
    char cm_load_cstr[128];
    char cm_replay_cstr[128];
    char cm_score_cstr[128];
    char cm_checkflashlight_cstr[128];
    char cm_updateflashlight_cstr[128];
    char cm_checktime_cstr[128];
    char cm_updatevariables_cstr[128];
    if (sscanf(line, "ar_lock=%d", &ar_lock_i) == 1)                               ar_parameter.lock = ar_lock_i;
    else if (sscanf(line, "ar_value=%f", &ar_value_f) == 1)                        ar_parameter.value = ar_value_f;
    else if (sscanf(line, "cs_lock=%d", &cs_lock_i) == 1)                          cs_parameter.lock = cs_lock_i;
    else if (sscanf(line, "cs_value=%f", &cs_value_f) == 1)                        cs_parameter.value = cs_value_f;
    else if (sscanf(line, "od_lock=%d", &od_lock_i) == 1)                          od_parameter.lock = od_lock_i;
    else if (sscanf(line, "od_value=%f", &od_value_f) == 1)                        od_parameter.value = od_value_f;
    else if (sscanf(line, "visible=%d", &mod_menu_visible_i) == 1)                 cfg_mod_menu_visible = mod_menu_visible_i;
    else if (sscanf(line, "font_size=%d", &font_size_i) == 1)                      cfg_font_size = font_size_i;
    else if (sscanf(line, "relax=%d", &relax_lock_i) == 1)                         cfg_relax_lock = relax_lock_i;
    else if (sscanf(line, "relax_style=%c", &relax_style_c) == 1)                  cfg_relax_style = (int)relax_style_c;
    else if (sscanf(line, "relax_checks_od=%d", &relax_checks_od_i) == 1)          cfg_relax_checks_od = relax_checks_od_i;
    else if (sscanf(line, "aimbot=%d", &aimbot_lock_i) == 1)                       cfg_aimbot_lock = aimbot_lock_i;
    else if (sscanf(line, "spins_per_minute=%d", &spins_per_minute_i) == 1)        cfg_spins_per_minute = spins_per_minute_i;
    else if (sscanf(line, "fraction_modifier=%f", &fraction_modifier_f) == 1)      cfg_fraction_modifier = fraction_modifier_f;
    else if (sscanf(line, "replay=%d", &replay_i) == 1)                            cfg_replay_enabled = replay_i;
    else if (sscanf(line, "replay_aim=%d", &replay_aim_i) == 1)                    cfg_replay_aim = replay_aim_i;
    else if (sscanf(line, "replay_keys=%d", &replay_keys_i) == 1)                  cfg_replay_keys = replay_keys_i;
    else if (sscanf(line, "sm_lock=%d", &score_multiplier_i) == 1)                 cfg_score_multiplier_enabled = score_multiplier_i;
    else if (sscanf(line, "sm_value=%f", &score_multiplier_value_f) == 1)          cfg_score_multiplier_value = score_multiplier_value_f;
    else if (sscanf(line, "drpc=%d", &discord_rich_presence_enabled_i) == 1)       cfg_discord_rich_presence_enabled = discord_rich_presence_enabled_i;
    else if (sscanf(line, "fl=%d", &flashlight_enabled_i) == 1)                    cfg_flashlight_enabled = flashlight_enabled_i;
    else if (sscanf(line, "hd=%d", &hidden_remover_enabled_i) == 1)                cfg_hidden_remover_enabled = hidden_remover_enabled_i;
    else if (sscanf(line, "tw_lock=%d", &timewarp_enabled_i) == 1)                 cfg_timewarp_enabled = timewarp_enabled_i;
    else if (sscanf(line, "tw_value=%lf", &timewarp_playback_rate_d) == 1)         cfg_timewarp_playback_rate = timewarp_playback_rate_d;
    else if (sscanf(line, "jump_window=%d", &jump_window_i) == 1)                  cfg_jumping_window = jump_window_i;
    else if (sscanf(line, "CSLoad=%s", cm_load_cstr) == 1)                         cm_load_s = std::string(cm_load_cstr);
    else if (sscanf(line, "CSReplay=%s", cm_replay_cstr) == 1)                     cm_replay_s = std::string(cm_replay_cstr);
    else if (sscanf(line, "CSScore=%s", cm_score_cstr) == 1)                       cm_score_s = std::string(cm_score_cstr);
    else if (sscanf(line, "CSCheckFlashlight=%s", cm_checkflashlight_cstr) == 1)   cm_checkflashlight_s = std::string(cm_checkflashlight_cstr);
    else if (sscanf(line, "CSUpdateFlashlight=%s", cm_updateflashlight_cstr) == 1) cm_updateflashlight_s = std::string(cm_updateflashlight_cstr);
    else if (sscanf(line, "CSCheckTime=%s", cm_checktime_cstr) == 1)               cm_checktime_s = std::string(cm_checktime_cstr);
    else if (sscanf(line, "CSUpdateVariables=%s", cm_updatevariables_cstr) == 1)   cm_updatevariables_s = std::string(cm_updatevariables_cstr);
}

void set_imgui_ini_handler()
{
    ImGuiSettingsHandler ini_handler;
    ini_handler.TypeName = "Config";
    ini_handler.TypeHash = ImHashStr("Config");
    ini_handler.ClearAllFn = ConfigHandler_ClearAll;
    ini_handler.ReadOpenFn = ConfigHandler_ReadOpen;
    ini_handler.ReadLineFn = ConfigHandler_ReadLine;
    ini_handler.ApplyAllFn = ConfigHandler_ApplyAll;
    ini_handler.WriteAllFn = ConfigHandler_WriteAll;
    ImGui::AddSettingsHandler(&ini_handler);
}
