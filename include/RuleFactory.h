#pragma once

#include "Config.h"

#include <memory>
#include <string>
#include <vector>

class Rule;

class RuleFactory {
public:
    // Returns nullptr if ruleId is unknown.
    static std::unique_ptr<Rule> createById(const std::string& ruleId);

    // Creates all built-in rules in a stable, documented order.
    std::vector<std::unique_ptr<Rule>> createAllRules() const;

    // Factory Method: creates rules from config (uses config.enabledRules if provided).
    std::vector<std::unique_ptr<Rule>> createFromConfig(const Config& config) const;

    // Backward-compatible alias.
    std::vector<std::unique_ptr<Rule>> createRules(const Config& config) const;

    // Stable list of all known rule IDs.
    static const std::vector<std::string>& allRuleIds();

    // Builds canonical enabled rule IDs from legacy boolean flags (stable order).
    static std::vector<std::string> enabledRuleIdsFromFlags(const Config& config);
};
