#include "rules/LineLengthRule.h"

#include "SeverityUtil.h"

namespace {

bool isBlank(const std::string& s)
{
    for (char c : s) {
        if (c != ' ' && c != '\t' && c != '\r') {
            return false;
        }
    }
    return true;
}

} 

void LineLengthRule::apply(const FileContext& file, const Config& config, AnalysisResult& result) const
{
    const int maxLen = config.maxLineLength;
    if (maxLen <= 0) {
        return;
    }

    const auto severity = configuredSeverity(config, id());

    for (std::size_t i = 0; i < file.lines.size(); ++i) {
        const auto& line = file.lines[i];
        if (isBlank(line)) {
            continue;
        }
        if (static_cast<int>(line.size()) > maxLen) {
            result.addIssue(Issue{severity, file.path, static_cast<int>(i + 1), maxLen + 1, id(),
                "Длина строки: " + std::to_string(line.size()) + " символов, лимит: " + std::to_string(maxLen) + ".",
                "Разбейте строку на несколько более коротких выражений или операторов."});
        }
    }
}
