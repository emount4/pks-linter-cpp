#pragma once

#include "Rule.h"

class NamingRule final : public Rule {
public:
    std::string id() const override { return "STYLE-NAMING"; }
    std::string name() const override { return "Именование"; }

    void apply(const FileContext& file, const Config& config, AnalysisResult& result) const override;
};
