#include "config.h"

void load_config(Config* cfg, std::string* err_msg) {
    const std::filesystem::path cfg_path{CONFIG_LOCATION};

    toml::table raw_cfg;
    if (!std::filesystem::exists(cfg_path))
        return;

    //std::string_view path(CONFIG_LOCATION);
    toml::parse_result result = toml::parse_file(std::string_view(CONFIG_LOCATION));
    if (!result) {
        *err_msg = "Could not parse config.toml";
        return;
    }
    raw_cfg = std::move(result).table();
    cfg->show_line_number = raw_cfg["general"]["show_line_number"].value_or(false);
    cfg->tab_spaces = raw_cfg["general"]["tab_spaces"].value_or(4);
    for (int i = 0; i < CONFIG_LATEST_COUNT; i++) {
        auto tmp_path = raw_cfg["latest"][std::to_string(i)].value_or("");
        cfg->latest[i] = std::filesystem::path(tmp_path);
        if (!std::filesystem::exists(tmp_path))
            cfg->latest[i] = std::filesystem::path("");
    }
}