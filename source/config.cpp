#include "config.h"

void load_config(Config* cfg, std::string* err_msg) {
    toml::table raw_cfg = parse_config(err_msg);

    if (*err_msg != "")
        return;

    cfg->show_line_number = raw_cfg["general"]["show_line_number"].value_or(false);
    cfg->tab_spaces = raw_cfg["general"]["tab_spaces"].value_or(4);
    
    std::string latest_location = LATEST_LOCATION;
    File latest = open_file(latest_location);

    if (!latest.read_success) {
        save_latest(cfg);
        return;
    }
    int i = 0;
    for (auto file_iter = latest.lines.begin(); i < CONFIG_LATEST_COUNT; file_iter++) {
        if (file_iter == latest.lines.begin())
            continue;
        std::string tmp_str;
        auto line = *file_iter;
        for (auto ch_iter = line.begin(); ch_iter != line.end(); ch_iter++)
            tmp_str += *ch_iter;
        tmp_str.resize(tmp_str.length() - 1);
        cfg->latest[i] = std::filesystem::path(tmp_str);
        if (!std::filesystem::exists(cfg->latest[i]))
            cfg->latest[i] = std::filesystem::path("");
        i++;
    }
}

// adds path to latest, shifts others if no free slot
void update_latest(Config* cfg, std::string new_path) {
    std::filesystem::path tmp_path(new_path);

    auto found = std::find(std::begin(cfg->latest), std::end(cfg->latest), tmp_path);
    if (found != std::end(cfg->latest))
        return;

    for (int i = CONFIG_LATEST_COUNT - 1; i > 0; i--) {
        auto old_path = cfg->latest[i - 1];
        cfg->latest[i] = old_path;
    }
    cfg->latest[0] = tmp_path;

    save_latest(cfg);
}

void save_latest(Config* cfg) {
    std::string latest_location = LATEST_LOCATION;
    File latest = open_file(latest_location);
    std::vector<char> line;

    for (auto it = cfg->latest_comment.begin(); it != cfg->latest_comment.end(); it++) {
        line.push_back(*it);
    }

    // clear and create the first line
    latest.lines.clear();
    latest.add_line(line);

    for (int i = 0; i < CONFIG_LATEST_COUNT; i++) {
        auto cstring = cfg->latest[i].c_str();
        std::vector<char> latest_line;
        while (*cstring != '\0') {
            latest_line.push_back(*cstring);
            cstring++;
        }
        latest.add_line(latest_line);
    }
    write_to_file(latest.filename, latest);
}

toml::table parse_config(std::string* err_msg) {
    const std::filesystem::path cfg_path{CONFIG_LOCATION};

    toml::table raw_cfg;
    if (!std::filesystem::exists(cfg_path))
        return raw_cfg;

    toml::parse_result result = toml::parse_file(std::string_view(CONFIG_LOCATION));
    if (!result) {
        *err_msg = "Could not parse config.toml";
        return raw_cfg;
    }
    raw_cfg = std::move(result).table();
    return raw_cfg;
}