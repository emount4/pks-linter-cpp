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
    std::vector<IObserver*> observers_;
};
