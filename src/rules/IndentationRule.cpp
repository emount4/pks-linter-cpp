#include "rules/IndentationRule.h"

#include "SeverityUtil.h"

namespace {

// Проверяет, состоит ли строка только из пробелов и табуляции.
bool isBlank(const std::string& s)
{
    for (char c : s) {
        if (c != ' ' && c != '\t' && c != '\r') {
            return false;
        }
    }
    return true;
}

} // пространство имен

// Проверяет отступы каждой непустой строки файла.
void IndentationRule::apply(const FileContext& file, const Config& config, AnalysisResult& result) const
{
    if (config.indentSize <= 0) {
        return;
    }

    const auto severity = configuredSeverity(config, id());

    for (std::size_t i = 0; i < file.lines.size(); ++i) {
        const auto& line = file.lines[i];
        const int lineNo = static_cast<int>(i + 1);

        if (line.empty() || isBlank(line)) {
            continue;
        }

        int spaces = 0;
        int tabs = 0;
        std::size_t pos = 0;
        while (pos < line.size()) {
            const char c = line[pos];
            if (c == ' ') {
                ++spaces;
                ++pos;
            } else if (c == '\t') {
                ++tabs;
                ++pos;
            } else {
                break;
            }
        }

        if (!config.allowTabs && tabs > 0) {
            result.addIssue(Issue{severity, file.path, lineNo, 1, id(),
                "В отступе используется символ табуляции.",
                "Замените табуляцию пробелами или включите allow_tabs."});
        }

        if (tabs > 0 && spaces > 0) {
            result.addIssue(Issue{severity, file.path, lineNo, 1, id(),
                "В отступе смешаны пробелы и табуляция.",
                "Используйте один стиль отступов во всем файле."});
        }

        if (tabs == 0 && spaces > 0 && (spaces % config.indentSize) != 0) {
            result.addIssue(Issue{severity, file.path, lineNo, 1, id(),
                "Отступ не кратен настроенному indent_size (" + std::to_string(config.indentSize) + ").",
                "Выровняйте отступ по значению indent_size из конфигурации."});
        }
    }
}
