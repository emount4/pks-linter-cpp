#include "rules/IndentationRule.h"

#include <cctype>

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

} // namespace

void IndentationRule::apply(const FileContext& file, const Config& config, AnalysisResult& result) const
{
    if (config.indentSize <= 0) {
        return;
    }

    for (std::size_t i = 0; i < file.lines.size(); ++i) {
        const auto& line = file.lines[i];
        const int lineNo = static_cast<int>(i + 1);

        if (line.empty() || isBlank(line)) {
            continue;
        }

        int spaces = 0;
        int tabs = 0;
        int pos = 0;
        while (pos < static_cast<int>(line.size())) {
            char c = line[static_cast<std::size_t>(pos)];
            if (c == ' ') {
                ++spaces;
                ++pos;
                continue;
            }
            if (c == '\t') {
                ++tabs;
                ++pos;
                continue;
            }
            break;
        }

        if (!config.allowTabs && tabs > 0) {
            result.addIssue(Issue{Severity::Warning,
                file.path,
                lineNo,
                1,
                id(),
                "Используется табуляция в отступах.",
                "Замените табуляцию на пробелы."});
        }

        if (tabs > 0 && spaces > 0) {
            result.addIssue(Issue{Severity::Warning,
                file.path,
                lineNo,
                1,
                id(),
                "Смешаны пробелы и табуляция в отступах.",
                "Используйте один тип отступов (рекомендуется пробелы)."});
        }

        if (tabs == 0 && spaces > 0 && (spaces % config.indentSize) != 0) {
            result.addIssue(Issue{Severity::Warning,
                file.path,
                lineNo,
                1,
                id(),
                "Отступ не кратен размеру indent_size (" + std::to_string(config.indentSize) + ").",
                "Выровняйте отступы по кратности indent_size."});
        }
    }
}
