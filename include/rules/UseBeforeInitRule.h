#pragma once

#include "Rule.h"

class UseBeforeInitRule final : public Rule {
public:
    std::string id() const override { return "BUG-USE-BEFORE-INIT"; }
    std::string name() const override { return "Use before initialization"; }

    void apply(const FileContext& file, const Config& config, AnalysisResult& result) const override;
};
