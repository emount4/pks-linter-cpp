#include <catch2/catch_test_macros.hpp>

#include "RuleFactory.h"
#include "Rule.h"

// Проверяет поддержку канонических id правил и коротких алиасов.
TEST_CASE("RuleFactory supports canonical ids and aliases")
{
    REQUIRE(RuleFactory::createById("STYLE-INDENTATION") != nullptr);
    REQUIRE(RuleFactory::createById("spacing") != nullptr);
    REQUIRE(RuleFactory::createById("line-length") != nullptr);
    REQUIRE(RuleFactory::createById("naming") != nullptr);
    REQUIRE(RuleFactory::createById("use-before-init") != nullptr);
    REQUIRE(RuleFactory::createById("memory-leak") != nullptr);
    REQUIRE(RuleFactory::createById("unknown-rule") == nullptr);
}

// Проверяет, что фабрика не создает отключенные правила.
TEST_CASE("RuleFactory ignores disabled rules")
{
    Config config;
    config.enabledRules = {"STYLE-INDENTATION", "STYLE-SPACING"};
    config.disabledRuleIds.insert("STYLE-SPACING");

    RuleFactory factory;
    auto rules = factory.createFromConfig(config);
    REQUIRE(rules.size() == 1);
    REQUIRE(rules[0]->id() == "STYLE-INDENTATION");
}
