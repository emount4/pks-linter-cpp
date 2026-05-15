#pragma once

#include "Config.h"
#include "ConfigLoader.h"

#include <filesystem>
#include <optional>
#include <string>

class ConfigManager {
public:
    static ConfigManager& instance()
    {
        static ConfigManager inst;
        return inst;
    }

    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ConfigManager(ConfigManager&&) = delete;
    ConfigManager& operator=(ConfigManager&&) = delete;

    std::optional<std::string> loadFromFile(const std::filesystem::path& path)
    {
        auto loaded = ConfigLoader::loadFromFile(path);
        config_ = std::move(loaded.config);
        return loaded.warning;
    }

    void set(Config config)
    {
        config_ = std::move(config);
    }

    const Config& get() const
    {
        return config_;
    }

private:
    ConfigManager() = default;

    Config config_{};
};
