#include <catch2/catch_test_macros.hpp>

#include "ConfigLoader.h"
#include "test_support.h"

TEST_CASE("ConfigLoader reads sectioned SRS config with severity and excludes")
{
    auto dir = makeTempDir("cfg_sectioned");
    auto file = dir / "config.ini";
    writeTextFile(file,
        "[general]\n"
        "mode=style\n"
        "show_progress=true\n"
        "[style]\n"
        "indent_size=2\n"
        "max_line_length=90\n"
        "variable_case=snake_case\n"
        "[rules]\n"
        "enabled=STYLE-INDENTATION,STYLE-SPACING\n"
        "disabled=STYLE-SPACING\n"
        "[severity]\n"
        "STYLE-INDENTATION=error\n"
        "UNKNOWN=critical\n"
        "[exclude]\n"
        "dirs=build,generated\n"
        "files=skip.cpp\n");

    auto loaded = ConfigLoader::loadFromFile(file);
    REQUIRE(loaded.config.mode == AnalysisMode::Style);
    REQUIRE(loaded.config.showProgress);
    REQUIRE(loaded.config.indentSize == 2);
    REQUIRE(loaded.config.maxLineLength == 90);
    REQUIRE(loaded.config.variableNaming == NamingStyle::LowerSnake);
    REQUIRE(loaded.config.enabledRules.size() == 2);
    REQUIRE(loaded.config.disabledRuleIds.count("STYLE-SPACING") == 1);
    REQUIRE(loaded.config.severityByRule["STYLE-INDENTATION"] == "error");
    REQUIRE(loaded.config.excludedFiles == std::vector<std::string>{"skip.cpp"});
}

TEST_CASE("ConfigLoader keeps defaults on missing file and invalid numbers")
{
    auto missing = ConfigLoader::loadFromFile("definitely_missing_config.ini");
    REQUIRE(missing.warning.has_value());
    REQUIRE(missing.config.maxLineLength == 120);

    auto dir = makeTempDir("cfg_bad_number");
    auto file = dir / "config.ini";
    writeTextFile(file, "[style]\nindent_size=oops\nmax_line_length=-1\n");
    auto loaded = ConfigLoader::loadFromFile(file);
    REQUIRE(loaded.config.indentSize == Config{}.indentSize);
    REQUIRE(loaded.config.maxLineLength == Config{}.maxLineLength);
    REQUIRE_FALSE(loaded.diagnostics.empty());
}
