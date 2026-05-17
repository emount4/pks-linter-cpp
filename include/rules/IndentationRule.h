#pragma once

#include "Rule.h"

class IndentationRule final : public Rule {
public:
    std::string id() const override { return "STYLE-INDENTATION"; }
    std::string name() const override { return "Отступы"; }

    void apply(const FileContext& file, const Config& config, AnalysisResult& result) const override;
};
