#pragma once

#include "Rule.h"

class UseBeforeInitRule final : public Rule {
public:
    // Возвращает id правила использования до инициализации.
    std::string id() const override { return "BUG-USE-BEFORE-INIT"; }

    // Возвращает название правила использования до инициализации.
    std::string name() const override { return "Использование до инициализации"; }

    // Ищет чтение локальных переменных до присваивания значения.
    void apply(const FileContext& file, const Config& config, AnalysisResult& result) const override;
};
