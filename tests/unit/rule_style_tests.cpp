#include <catch2/catch_test_macros.hpp>

#include "rules/IndentationRule.h"
#include "rules/LineLengthRule.h"
#include "rules/NamingRule.h"
#include "rules/SpacingRule.h"
#include "test_support.h"

TEST_CASE("Style rules cover spacing, indentation, line length, and naming")
{
    Config cfg;
    cfg.indentSize = 4;
    cfg.maxLineLength = 8;
    cfg.variableNaming = NamingStyle::LowerSnake;
    cfg.functionNaming = NamingStyle::LowerCamel;
    cfg.constantNaming = NamingStyle::UpperSnake;

    auto ctx = makeFileContextFromText("style.cpp",
        "\tint badName=1,b=2;\n"
        "int goodFunction(){ return badName+b; }\n"
        "#define bad_constant 1\n");

    AnalysisResult result;
    IndentationRule{}.apply(ctx, cfg, result);
    SpacingRule{}.apply(ctx, cfg, result);
    LineLengthRule{}.apply(ctx, cfg, result);
    NamingRule{}.apply(ctx, cfg, result);

    REQUIRE(result.warningCount() >= 5);
}

TEST_CASE("SpacingRule ignores unary minus and increment operators")
{
    auto ctx = makeFileContextFromText("spacing_ok.cpp", "int main(){ int a = -1; ++a; --a; return a; }\n");
    Config cfg;
    AnalysisResult result;
    SpacingRule{}.apply(ctx, cfg, result);
    REQUIRE(result.issues.empty());
}
