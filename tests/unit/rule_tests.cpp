#include <catch2/catch_test_macros.hpp>

#include "Config.h"
#include "Tokenizer.h"
#include "rules/IndentationRule.h"
#include "rules/LineLengthRule.h"
#include "rules/MemoryLeakRule.h"
#include "rules/NamingRule.h"
#include "rules/SpacingRule.h"
#include "rules/UseBeforeInitRule.h"
#include "test_support.h"

// Проверяет поиск табуляции и некратных отступов.
TEST_CASE("IndentationRule flags tabs and indentation size mismatches")
{
    auto ctx = makeFileContextFromText("indent.cpp",
        "\tint a = 0;\n"
        "  int b = 1;\n");

    Config cfg;
    cfg.indentationEnabled = true;
    cfg.allowTabs = false;
    cfg.indentSize = 4;

    AnalysisResult result;
    IndentationRule rule;
    rule.apply(ctx, cfg, result);

    REQUIRE(result.warningCount() == 2);
    REQUIRE(result.errorCount() == 0);
    REQUIRE(result.issues[0].ruleId == "STYLE-INDENTATION");
}

// Проверяет, что правило длины строки срабатывает только при превышении лимита.
TEST_CASE("LineLengthRule flags only lines longer than the configured maximum")
{
    auto ctx = makeFileContextFromText("length.cpp",
        "short\n"
        "12345678901\n");

    Config cfg;
    cfg.maxLineLength = 10;

    AnalysisResult result;
    LineLengthRule rule;
    rule.apply(ctx, cfg, result);

    REQUIRE(result.warningCount() == 1);
    REQUIRE(result.issues[0].line == 2);
    REQUIRE(result.issues[0].column == 11);
}

// Проверяет отсутствие пробелов вокруг операторов и после запятых.
TEST_CASE("SpacingRule flags missing spaces around operators and commas")
{
    auto ctx = makeFileContextFromText("spacing.cpp",
        "int main(){int a=1,b=2; return a+b;}\n");

    Config cfg;
    AnalysisResult result;
    SpacingRule rule;
    rule.apply(ctx, cfg, result);

    REQUIRE(result.warningCount() == 4);
    for (const auto& issue : result.issues) {
        REQUIRE(issue.ruleId == "STYLE-SPACING");
    }
}

// Проверяет нарушения именования констант, функций и переменных.
TEST_CASE("NamingRule flags constant, function and variable naming violations")
{
    auto ctx = makeFileContextFromText("naming.cpp",
        "#define max_value 10\n"
        "int Bad_function_name(){ int BadVariable = 0; return BadVariable; }\n");

    Config cfg;
    cfg.namingEnabled = true;
    cfg.variableNaming = NamingStyle::LowerCamel;
    cfg.functionNaming = NamingStyle::LowerCamel;
    cfg.constantNaming = NamingStyle::UpperSnake;

    AnalysisResult result;
    NamingRule rule;
    rule.apply(ctx, cfg, result);

    REQUIRE(result.warningCount() >= 3);
    REQUIRE(result.issues[0].ruleId == "STYLE-NAMING");
}

// Проверяет обнаружение чтения переменной до инициализации.
TEST_CASE("UseBeforeInitRule flags reads before initialization")
{
    auto ctx = makeFileContextFromText("use_before.cpp",
        "int main(){ int x; return x; }\n");

    Config cfg;
    AnalysisResult result;
    UseBeforeInitRule rule;
    rule.apply(ctx, cfg, result);

    REQUIRE(result.errorCount() == 1);
    REQUIRE(result.issues[0].ruleId == "BUG-USE-BEFORE-INIT");
}

// Проверяет, что инициализированные переменные не дают ложных срабатываний.
TEST_CASE("UseBeforeInitRule does not flag initialized variables")
{
    auto ctx = makeFileContextFromText("use_before_ok.cpp",
        "int main(){ int x = 0; return x; }\n");

    Config cfg;
    AnalysisResult result;
    UseBeforeInitRule rule;
    rule.apply(ctx, cfg, result);

    REQUIRE(result.issues.empty());
}

// Проверяет поиск new без delete и корректную пару new/delete.
TEST_CASE("MemoryLeakRule flags allocations without delete and accepts matched delete")
{
    auto leakCtx = makeFileContextFromText("leak.cpp",
        "int main(){ int* p = new int; return 0; }\n");
    auto okCtx = makeFileContextFromText("leak_ok.cpp",
        "int main(){ int* p = new int; delete p; return 0; }\n");

    Config cfg;
    AnalysisResult leakResult;
    AnalysisResult okResult;
    MemoryLeakRule rule;
    rule.apply(leakCtx, cfg, leakResult);
    rule.apply(okCtx, cfg, okResult);

    REQUIRE(leakResult.warningCount() == 1);
    REQUIRE(leakResult.issues[0].ruleId == "BUG-MEMORY-LEAK");
    REQUIRE(okResult.issues.empty());
}
