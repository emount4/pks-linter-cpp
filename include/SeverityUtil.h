#pragma once

#include "Config.h"
#include "Issue.h"

#include <string>

// Возвращает уровень серьезности по умолчанию для правила.
inline Severity defaultSeverityForRule(const std::string& ruleId)
{
    return ruleId == "BUG-USE-BEFORE-INIT" ? Severity::Error : Severity::Warning;
}

// Возвращает уровень серьезности из config или значение по умолчанию.
inline Severity configuredSeverity(const Config& config, const std::string& ruleId)
{
    auto it = config.severityByRule.find(ruleId);
    if (it == config.severityByRule.end()) {
        return defaultSeverityForRule(ruleId);
    }
    return it->second == "error" ? Severity::Error : Severity::Warning;
}
