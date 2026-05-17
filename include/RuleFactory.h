#pragma once

#include "Config.h"

#include <memory>
#include <string>
#include <vector>

class Rule;

class RuleFactory {
public:
    // Возвращает nullptr, если ruleId неизвестен.
    static std::unique_ptr<Rule> createById(const std::string& ruleId);

    // Создает все встроенные правила в стабильном задокументированном порядке.
    std::vector<std::unique_ptr<Rule>> createAllRules() const;

    // Фабричный метод: создает правила из config (использует config.enabledRules, если они заданы).
    std::vector<std::unique_ptr<Rule>> createFromConfig(const Config& config) const;

    // Алиас для обратной совместимости.
    std::vector<std::unique_ptr<Rule>> createRules(const Config& config) const;

    // Стабильный список всех известных идентификаторов правил.
    static const std::vector<std::string>& allRuleIds();

    // Строит канонические id включенных правил из старых boolean-флагов в стабильном порядке.
    static std::vector<std::string> enabledRuleIdsFromFlags(const Config& config);
};
