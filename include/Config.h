#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum class AnalysisMode {
    Style,
    Full,
};

enum class NamingStyle {
    Any,
    LowerCamel,
    UpperCamel,
    LowerSnake,
    UpperSnake,
};

struct Config {
    AnalysisMode mode{AnalysisMode::Full};
    bool showProgress{false};

    // Сканирование файлов
    std::vector<std::string> extensions{".cpp", ".h", ".hpp"};
    std::vector<std::string> excludedDirs{"build", ".git", "third_party", "cmake-build-debug", "cmake-build-release"};
    std::vector<std::string> excludedFiles;

    // Стилевые правила
    bool indentationEnabled{true};
    int indentSize{4};
    bool allowTabs{false};

    bool lineLengthEnabled{true};
    int maxLineLength{120};

    bool spacingEnabled{true};

    bool namingEnabled{true};
    NamingStyle variableNaming{NamingStyle::LowerSnake};
    NamingStyle functionNaming{NamingStyle::LowerCamel};
    NamingStyle constantNaming{NamingStyle::UpperSnake};

    // Правила поиска типовых ошибок
    bool useBeforeInitEnabled{true};
    bool memoryLeakEnabled{true};

    std::vector<std::string> enabledRules;

    // Переопределения из командной строки
    std::unordered_set<std::string> disabledRuleIds;

    // Уровни серьезности правил. Для отсутствующих правил используются значения по умолчанию.
    std::unordered_map<std::string, std::string> severityByRule;
};
