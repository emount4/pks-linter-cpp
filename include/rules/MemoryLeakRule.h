#pragma once

#include "Rule.h"

class MemoryLeakRule final : public Rule {
public:
    std::string id() const override { return "BUG-MEMORY-LEAK"; }
    std::string name() const override { return "Потенциальная утечка памяти"; }

    void apply(const FileContext& file, const Config& config, AnalysisResult& result) const override;
};
