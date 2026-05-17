#pragma once

#include "AnalysisResult.h"
#include "Config.h"

#include <filesystem>
#include <vector>

class IObserver;

class AnalyzerEngine {
public:
    void addObserver(IObserver* obs);
    AnalysisResult analyzeProject(const std::filesystem::path& root, const Config& config) const;

private:
    void notifyFileStart(const std::string& path) const;
    void notifyIssue(const Issue& issue) const;
    void notifyFileEnd(const std::string& path) const;
    void notifyAnalysisFinished(const AnalysisResult& result) const;

    std::vector<IObserver*> observers_;
};
