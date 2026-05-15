#pragma once

#include "Config.h"

#include <filesystem>
#include <optional>
#include <string>

struct ConfigLoadResult {
    Config config;
    std::optional<std::string> warning;
};

class ConfigLoader {
public:
    static ConfigLoadResult loadFromFile(const std::filesystem::path& path);
    static bool saveToFile(const Config& config, const std::filesystem::path& path, std::string* error);
};
