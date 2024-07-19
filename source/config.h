#pragma once

#include "toml.hpp"
#include <filesystem>
#include <string_view>

#define CONFIG_LOCATION "./config.toml"

// Stores config parameters from config.toml
struct Config {
    bool show_line_number = false;
};

void load_config(Config* cfg, std::string* err_msg);