#pragma once

#include "IObserver.h"

#include <string>
#include <vector>

struct RecordingObserver final : IObserver {
    std::vector<std::string> starts;
    std::vector<std::string> ends;
    std::vector<IssueEvent> issues;
    int finishedCount{0};

    void onIssue(const IssueEvent& event) override
    {
        issues.push_back(event);
    }

    void onFileStart(const std::string& path) override
    {
        starts.push_back(path);
    }

    void onFileEnd(const std::string& path) override
    {
        ends.push_back(path);
    }

    void onAnalysisFinished(const AnalysisResult&) override
    {
        ++finishedCount;
    }
};
