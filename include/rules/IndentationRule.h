#pragma once

#include "Rule.h"

class IndentationRule final : public Rule {
public:
    // Возвращает id правила проверки отступов.
    std::string id() const override { return "STYLE-INDENTATION"; }

    // Возвращает название правила проверки отступов.
    std::string name() const override { return "Отступы"; }

    // Проверяет отступы, табуляцию и смешивание пробелов с табами.
    void apply(const FileContext& file, const Config& config, AnalysisResult& result) const override;
};
