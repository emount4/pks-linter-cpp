#pragma once

#include <string>
#include <unordered_set>
#include <vector>

enum class NamingStyle {
    Any,
    LowerCamel,
    UpperCamel,
    LowerSnake,
    UpperSnake,
};

struct Config {
    // File scanning
    std::vector<std::string> extensions{".cpp", ".h", ".hpp"};
    std::vector<std::string> excludedDirs;

    // Style rules
    bool indentationEnabled{true};
    int indentSize{4};
    bool allowTabs{false};

    bool lineLengthEnabled{true};
    int maxLineLength{80};

    bool spacingEnabled{true};

    bool namingEnabled{true};
    NamingStyle variableNaming{NamingStyle::LowerCamel};
    NamingStyle functionNaming{NamingStyle::LowerCamel};
    NamingStyle constantNaming{NamingStyle::UpperSnake};

    // Bug rules
    bool useBeforeInitEnabled{true};
    bool memoryLeakEnabled{true};

    std::vector<std::string> enabledRules;

    // CLI overrides
    std::unordered_set<std::string> disabledRuleIds;
};
