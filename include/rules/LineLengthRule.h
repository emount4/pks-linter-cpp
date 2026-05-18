#pragma once

#include "Rule.h"

class LineLengthRule final : public Rule {
public:
    // Возвращает id правила проверки длины строки.
    std::string id() const override { return "STYLE-LINE-LENGTH"; }

    // Возвращает название правила проверки длины строки.
    std::string name() const override { return "Длина строки"; }

    // Проверяет превышение максимальной длины строки.
    void apply(const FileContext& file, const Config& config, AnalysisResult& result) const override;
};
