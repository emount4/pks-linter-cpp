#pragma once

#include "Config.h"

#include <filesystem>
#include <istream>
#include <ostream>

struct InteractiveSessionResult {
    std::filesystem::path projectPath;
    std::filesystem::path saveConfigPath;
    Config config;
};

class InteractiveConfig {
public:
    static InteractiveSessionResult collect(std::istream& input, std::ostream& output, Config defaults = {});
};
