#pragma once

#include "Rule.h"

class LineLengthRule final : public Rule {
public:
    std::string id() const override { return "STYLE-LINE-LENGTH"; }
    std::string name() const override { return "Длина строки"; }

    void apply(const FileContext& file, const Config& config, AnalysisResult& result) const override;
};
