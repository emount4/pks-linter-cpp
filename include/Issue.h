#pragma once

#include <filesystem>
#include <string>

enum class Severity {
    Error,
    Warning,
};

struct Issue {
    Severity severity{Severity::Warning};
    std::filesystem::path file;
    int line{0};
    int column{0};
    std::string ruleId;
    std::string message;
    std::string recommendation;
    std::string ruleName;
};
