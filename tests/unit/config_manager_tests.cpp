#include <catch2/catch_test_macros.hpp>

#include "ConfigManager.h"
#include "test_support.h"

// Проверяет основные операции одиночки конфигурации.
TEST_CASE("ConfigManager can load save set get and reset config")
{
    auto& manager = ConfigManager::instance();
    manager.reset();
    Config cfg;
    cfg.indentSize = 6;
    manager.set(cfg);
    REQUIRE(manager.get().indentSize == 6);

    auto dir = makeTempDir("manager_roundtrip");
    auto file = dir / "config.ini";
    std::string error;
    REQUIRE(manager.saveToFile(file, &error));
    REQUIRE(error.empty());

    manager.reset();
    REQUIRE(manager.get().indentSize == Config{}.indentSize);
    REQUIRE_FALSE(manager.loadFromFile(file).has_value());
    REQUIRE(manager.get().indentSize == 6);
}
