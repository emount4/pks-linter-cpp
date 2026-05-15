#pragma once

#include "Issue.h"

#include <cstddef>
#include <functional>
#include <vector>

struct AnalysisResult {
    std::vector<Issue> issues;
    std::size_t filesChecked{0};

    void setIssueCallback(std::function<void(const Issue&)> cb)
    {
        onIssue_ = std::move(cb);
    }

    void addIssue(Issue issue)
    {
        issues.push_back(std::move(issue));
        if (onIssue_) {
            onIssue_(issues.back());
        }
    }

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
    std::function<void(const Issue&)> onIssue_{};
};
