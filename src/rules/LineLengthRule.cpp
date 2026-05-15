#include "rules/LineLengthRule.h"

void LineLengthRule::apply(const FileContext& file, const Config& config, AnalysisResult& result) const
{
    const int maxLen = config.maxLineLength;
    if (maxLen <= 0) {
        return;
    }

    for (std::size_t i = 0; i < file.lines.size(); ++i) {
        const auto& line = file.lines[i];
        if (static_cast<int>(line.size()) > maxLen) {
            result.addIssue(Issue{Severity::Warning,
                file.path,
                static_cast<int>(i + 1),
                maxLen + 1,
                id(),
                "Строка длиной " + std::to_string(line.size()) + " символов (максимум " + std::to_string(maxLen) + ").",
                "Разбейте строку на несколько более коротких."});
        }
    }
}
