#include "RuleFactory.h"

#include "Rule.h"

#include "rules/IndentationRule.h"
#include "rules/LineLengthRule.h"
#include "rules/MemoryLeakRule.h"
#include "rules/NamingRule.h"
#include "rules/SpacingRule.h"
#include "rules/UseBeforeInitRule.h"

namespace {

constexpr const char* kStyleIndent = "STYLE-INDENT";
constexpr const char* kStyleSpacing = "STYLE-SPACING";
constexpr const char* kStyleLineLength = "STYLE-LINE-LENGTH";
constexpr const char* kStyleNaming = "STYLE-NAMING";
constexpr const char* kBugUseBeforeInit = "BUG-USE-BEFORE-INIT";
constexpr const char* kBugMemoryLeak = "BUG-MEMORY-LEAK";

std::string normalizeRuleId(std::string id)
{
    // trim spaces
    while (!id.empty() && (id.front() == ' ' || id.front() == '\t')) {
        id.erase(id.begin());
    }
    while (!id.empty() && (id.back() == ' ' || id.back() == '\t')) {
        id.pop_back();
    }

    // Allow both canonical ids and short aliases (as in the report).
    if (id == "indentation") {
        return kStyleIndent;
    }
    if (id == "spacing") {
        return kStyleSpacing;
    }
    if (id == "line-length" || id == "line_length" || id == "lineLength") {
        return kStyleLineLength;
    }
    if (id == "naming") {
        return kStyleNaming;
    }
    if (id == "use-before-init" || id == "use_before_init" || id == "useBeforeInit") {
        return kBugUseBeforeInit;
    }
    if (id == "memory-leak" || id == "memory_leak" || id == "memoryLeak") {
        return kBugMemoryLeak;
    }
    return id;
}

} // namespace

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

    std::vector<std::string> idsToCreate;
    if (!config.enabledRules.empty()) {
        idsToCreate = config.enabledRules;
    } else {
        idsToCreate = enabledRuleIdsFromFlags(config);
    }

    for (auto id : idsToCreate) {
        id = normalizeRuleId(std::move(id));
        if (config.disabledRuleIds.find(id) != config.disabledRuleIds.end()) {
            continue;
        }
        if (auto r = createById(id)) {
            out.push_back(std::move(r));
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
