#pragma once

#include "toml.hpp"
#include "file.h"
#include "file_io.h"
#include <filesystem>
#include <string_view>
#include <algorithm>
#include <iostream>

#define CONFIG_LOCATION "./config.toml"
#define LATEST_LOCATION "./latest"
#define CONFIG_LATEST_COUNT 4

// Stores config parameters from config.toml
struct Config {
    const std::string latest_comment = "# Auto generated routes of latest opened files";
    bool show_line_number = false;
    unsigned int tab_spaces = 4;
    std::filesystem::path latest[CONFIG_LATEST_COUNT];
};

void load_config(Config* cfg, std::string* err_msg);

void update_latest(Config* cfg, std::string new_path);

void save_latest(Config* cfg);

toml::table parse_config(std::string* err_msg);