#pragma once

#include "Issue.h"

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

struct AnalysisResult {
    std::vector<Issue> issues;
    std::size_t filesChecked{0};
    std::size_t skippedFiles{0};
    std::size_t failedFiles{0};

    // Сохраняет callback, который вызывается при добавлении нового нарушения.
    void setIssueCallback(std::function<void(const Issue&)> cb)
    {
        onIssue_ = std::move(cb);
    }

    // Добавляет нарушение в результат и уведомляет подписчика, если он задан.
    void addIssue(Issue issue)
    {
        if (issue.ruleName.empty()) {
            issue.ruleName = ruleNameForId(issue.ruleId);
        }
        issues.push_back(std::move(issue));
        if (onIssue_) {
            onIssue_(issues.back());
        }
    }

    // Возвращает количество нарушений уровня "ошибка".
    std::size_t errorCount() const
    {
        std::size_t count = 0;
        for (const auto& issue : issues) {
            if (issue.severity == Severity::Error) {
                ++count;
            }
        }
        return count;
    }

    // Возвращает количество нарушений уровня "предупреждение".
    std::size_t warningCount() const
    {
        std::size_t count = 0;
        for (const auto& issue : issues) {
            if (issue.severity == Severity::Warning) {
                ++count;
            }
        }
        return count;
    }

private:
    // Подставляет человекочитаемое имя правила по его id.
    static std::string ruleNameForId(const std::string& ruleId)
    {
        if (ruleId == "STYLE-INDENTATION") {
            return "Отступы";
        }
        if (ruleId == "STYLE-SPACING") {
            return "Пробелы";
        }
        if (ruleId == "STYLE-LINE-LENGTH") {
            return "Длина строки";
        }
        if (ruleId == "STYLE-NAMING") {
            return "Именование";
        }
        if (ruleId == "BUG-USE-BEFORE-INIT") {
            return "Использование до инициализации";
        }
        if (ruleId == "BUG-MEMORY-LEAK") {
            return "Потенциальная утечка памяти";
        }
        return ruleId;
    }

    std::function<void(const Issue&)> onIssue_{};
};
