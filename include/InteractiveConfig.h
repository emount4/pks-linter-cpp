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
    // Собирает настройки интерактивного режима из потоков ввода и вывода.
    static InteractiveSessionResult collect(std::istream& input, std::ostream& output, Config defaults = {});
};
