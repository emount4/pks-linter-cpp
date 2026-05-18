#pragma once

#include "Rule.h"

class SpacingRule final : public Rule {
public:
    // Возвращает id правила проверки пробелов.
    std::string id() const override { return "STYLE-SPACING"; }

    // Возвращает название правила проверки пробелов.
    std::string name() const override { return "Пробелы"; }

    // Проверяет пробелы вокруг операторов и после запятых.
    void apply(const FileContext& file, const Config& config, AnalysisResult& result) const override;
};
