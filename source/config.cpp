#include "config.h"

void load_config(Config* cfg, std::string* err_msg) {
    const std::filesystem::path cfg_path{CONFIG_LOCATION};
    /*
    if (!std::filesystem::exists(cfg_path)) {
        *err_msg = "Could not find config.toml.";
        return;
    }
    */

    toml::table raw_cfg;
    if (std::filesystem::exists(cfg_path))
        try {
            std::string_view path{CONFIG_LOCATION};
            raw_cfg = toml::parse_file(path);
            cfg->show_line_number = raw_cfg["general"]["show_line_number"].value_or(false);
            cfg->tab_spaces = raw_cfg["general"]["tab_spaces"].value_or(4);
        } catch (const toml::parse_error& err) {
            *err_msg = "Could not parse config.toml.";
            return;
        }

    return;
}