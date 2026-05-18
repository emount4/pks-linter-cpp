#pragma once

#include "AnalysisResult.h"
#include "Config.h"
#include "FileContext.h"

#include <string>

class Rule {
public:
    // Обеспечивает корректное удаление правил через базовый интерфейс.
    virtual ~Rule() = default;

    // Возвращает стабильный идентификатор правила.
    virtual std::string id() const = 0;

    // Возвращает человекочитаемое имя правила.
    virtual std::string name() const = 0;

    // Применяет правило к контексту файла и добавляет найденные нарушения.
    virtual void apply(const FileContext& file, const Config& config, AnalysisResult& result) const = 0;
};
