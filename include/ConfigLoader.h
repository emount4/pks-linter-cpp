#pragma once

#include "Config.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

struct ConfigLoadResult {
    Config config;
    std::optional<std::string> warning;
    std::vector<std::string> diagnostics;
};

class ConfigLoader {
public:
    static ConfigLoadResult loadFromFile(const std::filesystem::path& path);
    static bool saveToFile(const Config& config, const std::filesystem::path& path, std::string* error);
};
