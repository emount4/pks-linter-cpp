#pragma once

#include "Config.h"
#include "ConfigLoader.h"

#include <filesystem>
#include <optional>
#include <string>

class ConfigManager {
public:
    // Возвращает единственный экземпляр менеджера конфигурации.
    static ConfigManager& instance()
    {
        static ConfigManager inst;
        return inst;
    }

    // Запрещает копирование единственного экземпляра менеджера.
    ConfigManager(const ConfigManager&) = delete;
    // Запрещает присваивание копированием для единственного экземпляра менеджера.
    ConfigManager& operator=(const ConfigManager&) = delete;
    // Запрещает перемещение единственного экземпляра менеджера.
    ConfigManager(ConfigManager&&) = delete;
    // Запрещает присваивание перемещением для единственного экземпляра менеджера.
    ConfigManager& operator=(ConfigManager&&) = delete;

    // Загружает конфигурацию из файла и сохраняет ее как текущую.
    std::optional<std::string> loadFromFile(const std::filesystem::path& path)
    {
        auto loaded = ConfigLoader::loadFromFile(path);
        config_ = std::move(loaded.config);
        return loaded.warning;
    }

    // Заменяет текущую конфигурацию переданным значением.
    void set(Config config)
    {
        config_ = std::move(config);
    }

    // Возвращает текущую конфигурацию.
    const Config& get() const
    {
        return config_;
    }

    // Сохраняет текущую конфигурацию в файл.
    bool saveToFile(const std::filesystem::path& path, std::string* error = nullptr) const
    {
        return ConfigLoader::saveToFile(config_, path, error);
    }

    // Сбрасывает конфигурацию к значениям по умолчанию.
    void reset()
    {
        config_ = Config{};
    }

private:
    // Закрывает создание объектов извне для реализации Singleton.
    ConfigManager() = default;

    Config config_{};
};
