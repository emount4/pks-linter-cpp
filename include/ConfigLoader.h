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
    // Загружает настройки из INI-файла или возвращает конфиг по умолчанию с диагностикой.
    static ConfigLoadResult loadFromFile(const std::filesystem::path& path);

    // Сохраняет текущую конфигурацию в INI-файл и возвращает статус записи.
    static bool saveToFile(const Config& config, const std::filesystem::path& path, std::string* error);
};
