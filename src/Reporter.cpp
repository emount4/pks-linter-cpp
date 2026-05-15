#include "Reporter.h"

#include <algorithm>
#include <iostream>

auto issueLess(const Issue& a, const Issue& b)
{
    if (a.file != b.file) {
        return a.file.generic_string() < b.file.generic_string();
    }
    if (a.line != b.line) {
        return a.line < b.line;
    }
    return a.column < b.column;
}

void Reporter::print(const AnalysisResult& result, std::ostream& os)
{
    std::vector<Issue> issues = result.issues;
    std::sort(issues.begin(), issues.end(), issueLess);

    for (const auto& issue : issues) {
        const char* sev = (issue.severity == Severity::Error) ? "ERROR" : "WARNING";
        os << '[' << sev << "] " << issue.file.generic_string() << ':' << issue.line;
        if (issue.column > 0) {
            os << ':' << issue.column;
        }
        os << ": " << issue.message << "\n";
        if (!issue.ruleId.empty()) {
            os << "        Rule: " << issue.ruleId << "\n";
        }
        if (!issue.recommendation.empty()) {
            os << "        Recommendation: " << issue.recommendation << "\n";
        }
        os << "\n";
    }

    os << "=== SUMMARY ===\n";
    os << "Files checked: " << result.filesChecked << "\n";
    os << "Errors: " << result.errorCount() << "\n";
    os << "Warnings: " << result.warningCount() << "\n";
}
