#pragma once

#include "Rule.h"

class MemoryLeakRule final : public Rule {
public:
    // Возвращает id правила поиска утечек памяти.
    std::string id() const override { return "BUG-MEMORY-LEAK"; }

    // Возвращает название правила поиска утечек памяти.
    std::string name() const override { return "Потенциальная утечка памяти"; }

    // Ищет простые несоответствия new/delete в пределах функции.
    void apply(const FileContext& file, const Config& config, AnalysisResult& result) const override;
};
