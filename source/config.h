#pragma once

#include "toml.hpp"
#include <filesystem>
#include <string_view>
#include <algorithm>

#define CONFIG_LOCATION "./config.toml"
#define CONFIG_LATEST_COUNT 4

// Stores config parameters from config.toml
struct Config {
    const unsigned int latest_count = 4;

    bool show_line_number = false;
    unsigned int tab_spaces = 4;
    std::filesystem::path latest[CONFIG_LATEST_COUNT];
};

void load_config(Config* cfg, std::string* err_msg);