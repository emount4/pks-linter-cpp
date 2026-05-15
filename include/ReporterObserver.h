#pragma once

#include "IObserver.h"
#include "Reporter.h"

#include <ostream>

class ReporterObserver final : public IObserver {
public:
    explicit ReporterObserver(std::ostream& os)
        : os_(os)
    {
    }

    void onIssue(const IssueEvent&) override {}
    void onFileStart(const std::string&) override {}
    void onFileEnd(const std::string&) override {}

    void onAnalysisFinished(const AnalysisResult& result) override
    {
        Reporter::print(result, os_);
    }

private:
    std::ostream& os_;
};
