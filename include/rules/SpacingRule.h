#pragma once

#include "Rule.h"

class SpacingRule final : public Rule {
public:
    std::string id() const override { return "STYLE-SPACING"; }
    std::string name() const override { return "Пробелы"; }

    void apply(const FileContext& file, const Config& config, AnalysisResult& result) const override;
};
