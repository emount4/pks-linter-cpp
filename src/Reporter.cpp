#include "Reporter.h"

#include <algorithm>

namespace {

bool issueLess(const Issue& a, const Issue& b)
{
    const auto af = a.file.lexically_normal().generic_string();
    const auto bf = b.file.lexically_normal().generic_string();
    if (af != bf) {
        return af < bf;
    }
    if (a.line != b.line) {
        return a.line < b.line;
    }
    if (a.column != b.column) {
        return a.column < b.column;
    }
    return a.ruleId < b.ruleId;
}

const char* severityText(Severity severity)
{
    return severity == Severity::Error ? "ошибка" : "предупреждение";
}

} // пространство имен

void Reporter::print(const AnalysisResult& result, std::ostream& os)
{
    std::vector<Issue> issues = result.issues;
    std::sort(issues.begin(), issues.end(), issueLess);

    if (issues.empty()) {
        os << "Нарушений не найдено.\n";
    }

    for (const auto& issue : issues) {
        os << issue.file.lexically_normal().generic_string() << ':' << issue.line << ':' << issue.column
           << ": " << severityText(issue.severity) << ": " << issue.ruleId << ": " << issue.message << '\n';
        if (!issue.recommendation.empty()) {
            os << "рекомендация: " << issue.recommendation << '\n';
        }
    }

    os << "проверено файлов: " << result.filesChecked << '\n';
    os << "предупреждений: " << result.warningCount() << '\n';
    os << "ошибок: " << result.errorCount() << '\n';
    os << "всего нарушений: " << result.issues.size() << '\n';
    if (result.skippedFiles > 0) {
        os << "пропущено файлов: " << result.skippedFiles << '\n';
    }
    if (result.failedFiles > 0) {
        os << "ошибок чтения файлов: " << result.failedFiles << '\n';
    }
}
