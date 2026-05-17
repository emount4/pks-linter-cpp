#include "RuleFactory.h"

#include "Rule.h"

#include "rules/IndentationRule.h"
#include "rules/LineLengthRule.h"
#include "rules/MemoryLeakRule.h"
#include "rules/NamingRule.h"
#include "rules/SpacingRule.h"
#include "rules/UseBeforeInitRule.h"

#include <cctype>
#include <iostream>
#include <unordered_set>

namespace {

constexpr const char* kStyleIndent = "STYLE-INDENTATION";
constexpr const char* kStyleSpacing = "STYLE-SPACING";
constexpr const char* kStyleLineLength = "STYLE-LINE-LENGTH";
constexpr const char* kStyleNaming = "STYLE-NAMING";
constexpr const char* kBugUseBeforeInit = "BUG-USE-BEFORE-INIT";
constexpr const char* kBugMemoryLeak = "BUG-MEMORY-LEAK";

std::string normalizeRuleId(std::string id)
{
    // Убираем пробелы по краям.
    while (!id.empty() && (id.front() == ' ' || id.front() == '\t')) {
        id.erase(id.begin());
    }
    while (!id.empty() && (id.back() == ' ' || id.back() == '\t')) {
        id.pop_back();
    }

    std::string lowered = id;
    for (auto& c : lowered) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    if (lowered == "style-indent" || lowered == "style-indentation" || lowered == "indentation") {
        return kStyleIndent;
    }
    if (lowered == "style-spacing" || lowered == "spacing") {
        return kStyleSpacing;
    }
    if (lowered == "style-line-length" || lowered == "line-length" || lowered == "line_length" || lowered == "linelength") {
        return kStyleLineLength;
    }
    if (lowered == "style-naming" || lowered == "naming") {
        return kStyleNaming;
    }
    if (lowered == "bug-use-before-init" || lowered == "use-before-init" || lowered == "use_before_init" || lowered == "usebeforeinit") {
        return kBugUseBeforeInit;
    }
    if (lowered == "bug-memory-leak" || lowered == "memory-leak" || lowered == "memory_leak" || lowered == "memoryleak") {
        return kBugMemoryLeak;
    }
    return id;
}

} 

std::unique_ptr<Rule> RuleFactory::createById(const std::string& ruleId)
{
    const auto id = normalizeRuleId(ruleId);
    if (id == kStyleIndent) {
        return std::make_unique<IndentationRule>();
    }
    if (id == kStyleSpacing) {
        return std::make_unique<SpacingRule>();
    }
    if (id == kStyleLineLength) {
        return std::make_unique<LineLengthRule>();
    }
    if (id == kStyleNaming) {
        return std::make_unique<NamingRule>();
    }
    if (id == kBugUseBeforeInit) {
        return std::make_unique<UseBeforeInitRule>();
    }
    if (id == kBugMemoryLeak) {
        return std::make_unique<MemoryLeakRule>();
    }
    return nullptr;
}

std::vector<std::unique_ptr<Rule>> RuleFactory::createAllRules() const
{
    std::vector<std::unique_ptr<Rule>> rules;

    for (const auto& id : allRuleIds()) {
        if (auto r = createById(id)) {
            rules.push_back(std::move(r));
        }
    }
    return rules;
}

std::vector<std::string> RuleFactory::enabledRuleIdsFromFlags(const Config& config)
{
    std::vector<std::string> ids;
    if (config.indentationEnabled) {
        ids.push_back(kStyleIndent);
    }
    if (config.spacingEnabled) {
        ids.push_back(kStyleSpacing);
    }
    if (config.lineLengthEnabled) {
        ids.push_back(kStyleLineLength);
    }
    if (config.namingEnabled) {
        ids.push_back(kStyleNaming);
    }
    if (config.useBeforeInitEnabled) {
        ids.push_back(kBugUseBeforeInit);
    }
    if (config.memoryLeakEnabled) {
        ids.push_back(kBugMemoryLeak);
    }
    return ids;
}

std::vector<std::unique_ptr<Rule>> RuleFactory::createFromConfig(const Config& config) const
{
    std::vector<std::unique_ptr<Rule>> out;
    std::unordered_set<std::string> disabled;
    for (auto id : config.disabledRuleIds) {
        disabled.insert(normalizeRuleId(std::move(id)));
    }

    std::vector<std::string> idsToCreate;
    if (!config.enabledRules.empty()) {
        idsToCreate = config.enabledRules;
    } else {
        idsToCreate = enabledRuleIdsFromFlags(config);
    }

    for (auto id : idsToCreate) {
        id = normalizeRuleId(std::move(id));
        if (disabled.find(id) != disabled.end()) {
            continue;
        }
        if (auto r = createById(id)) {
            out.push_back(std::move(r));
        } else {
            std::cerr << "[ПРЕДУПРЕЖДЕНИЕ] Неизвестное правило проигнорировано: " << id << '\n';
        }
    }

    return out;
}

std::vector<std::unique_ptr<Rule>> RuleFactory::createRules(const Config& config) const
{
    return createFromConfig(config);
}

const std::vector<std::string>& RuleFactory::allRuleIds()
{
    static const std::vector<std::string> ids = {
        kStyleIndent,
        kStyleSpacing,
        kStyleLineLength,
        kStyleNaming,
        kBugUseBeforeInit,
        kBugMemoryLeak,
    };
    return ids;
}
