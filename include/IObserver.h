#pragma once

#include "AnalysisResult.h"
#include "IssueEvent.h"

#include <string>

class IObserver {
public:
    virtual ~IObserver() = default;

    virtual void onIssue(const IssueEvent& event) = 0;
    virtual void onFileStart(const std::string& path) = 0;
    virtual void onFileEnd(const std::string& path) = 0;
    virtual void onAnalysisFinished(const AnalysisResult&) {}
};
