#include <catch2/catch_test_macros.hpp>

#include "ConfigLoader.h"
#include "ConfigManager.h"
#include "RuleFactory.h"
#include "rules/IndentationRule.h"
#include "rules/LineLengthRule.h"
#include "rules/MemoryLeakRule.h"
#include "rules/NamingRule.h"
#include "rules/SpacingRule.h"
#include "rules/UseBeforeInitRule.h"
#include "test_support.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

// Проверяет, что ConfigManager хранит единую активную конфигурацию.
TEST_CASE("ConfigManager is a singleton and stores one active config")
{
    auto& first = ConfigManager::instance();
    auto& second = ConfigManager::instance();
    REQUIRE(&first == &second);

    Config config;
    config.indentSize = 2;
    config.maxLineLength = 120;
    first.set(config);

    REQUIRE(second.get().indentSize == 2);
    REQUIRE(second.get().maxLineLength == 120);
}

// Проверяет чтение включенных правил и алиасов стилей именования из config.
TEST_CASE("ConfigLoader parses enabled rules and naming aliases")
{
    auto dir = makeTempDir("cfg_loader_parse");
    auto file = dir / "config.ini";

    writeTextFile(file,
        "extensions=.cpp,.hpp\n"
        "exclude.dirs=build,generated\n"
        "enabled.rules=indentation,spacing,line-length,naming,use-before-init,memory-leak\n"
        "style.naming.variables=snake_case\n"
        "style.naming.functions=PascalCase\n"
        "style.naming.constants=upper_snake\n");

    auto loaded = ConfigLoader::loadFromFile(file);
    REQUIRE_FALSE(loaded.warning.has_value());
    REQUIRE(loaded.config.extensions.size() == 2);
    REQUIRE(loaded.config.extensions[0] == ".cpp");
    REQUIRE(loaded.config.extensions[1] == ".hpp");
    REQUIRE(loaded.config.excludedDirs.size() == 2);
    REQUIRE(loaded.config.enabledRules.size() == 6);
    REQUIRE(loaded.config.variableNaming == NamingStyle::LowerSnake);
    REQUIRE(loaded.config.functionNaming == NamingStyle::UpperCamel);
    REQUIRE(loaded.config.constantNaming == NamingStyle::UpperSnake);
}

// Проверяет, что сохраненный config загружается обратно без потери основных настроек.
TEST_CASE("ConfigLoader save and load round-trips the main settings")
{
    auto dir = makeTempDir("cfg_loader_roundtrip");
    auto file = dir / "saved.ini";

    Config config;
    config.extensions = {".cpp", ".h", ".hpp"};
    config.excludedDirs = {"build", "dist"};
    config.enabledRules = {"STYLE-INDENTATION", "STYLE-SPACING", "BUG-MEMORY-LEAK"};
    config.indentationEnabled = false;
    config.lineLengthEnabled = true;
    config.maxLineLength = 100;

    std::string error;
    REQUIRE(ConfigLoader::saveToFile(config, file, &error));
    REQUIRE(error.empty());

    auto loaded = ConfigLoader::loadFromFile(file);
    REQUIRE_FALSE(loaded.warning.has_value());
    REQUIRE(loaded.config.extensions == config.extensions);
    REQUIRE(loaded.config.excludedDirs == config.excludedDirs);
    REQUIRE(loaded.config.enabledRules == config.enabledRules);
    REQUIRE_FALSE(loaded.config.indentationEnabled);
    REQUIRE(loaded.config.maxLineLength == 100);
}

// Проверяет создание встроенных правил по каноническим идентификаторам.
TEST_CASE("RuleFactory creates built-in rules by canonical id")
{
    auto rule = RuleFactory::createById("STYLE-INDENTATION");
    REQUIRE(rule != nullptr);
    REQUIRE(rule->id() == "STYLE-INDENTATION");
    REQUIRE(rule->name() == "Отступы");

    REQUIRE(RuleFactory::createById("UNKNOWN-RULE") == nullptr);
}

// Проверяет создание правил из списка enabled.rules с короткими алиасами.
TEST_CASE("RuleFactory creates rules from enabled.rules aliases")
{
    Config config;
    config.enabledRules = {"indentation", "spacing", "line-length", "naming", "use-before-init", "memory-leak"};

    RuleFactory factory;
    auto rules = factory.createFromConfig(config);
    REQUIRE(rules.size() == 6);
    REQUIRE(rules[0]->id() == "STYLE-INDENTATION");
    REQUIRE(rules[1]->id() == "STYLE-SPACING");
    REQUIRE(rules[2]->id() == "STYLE-LINE-LENGTH");
    REQUIRE(rules[3]->id() == "STYLE-NAMING");
    REQUIRE(rules[4]->id() == "BUG-USE-BEFORE-INIT");
    REQUIRE(rules[5]->id() == "BUG-MEMORY-LEAK");
}
