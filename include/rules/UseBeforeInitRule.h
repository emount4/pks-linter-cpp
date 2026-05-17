#pragma once

#include "Rule.h"

class UseBeforeInitRule final : public Rule {
public:
    std::string id() const override { return "BUG-USE-BEFORE-INIT"; }
    std::string name() const override { return "Использование до инициализации"; }

    void apply(const FileContext& file, const Config& config, AnalysisResult& result) const override;
};
