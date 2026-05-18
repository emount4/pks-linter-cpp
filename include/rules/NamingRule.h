#pragma once

#include "Rule.h"

class NamingRule final : public Rule {
public:
    // Возвращает id правила проверки именования.
    std::string id() const override { return "STYLE-NAMING"; }

    // Возвращает название правила проверки именования.
    std::string name() const override { return "Именование"; }

    // Проверяет имена функций, переменных и констант.
    void apply(const FileContext& file, const Config& config, AnalysisResult& result) const override;
};
