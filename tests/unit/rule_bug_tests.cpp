#include <catch2/catch_test_macros.hpp>

#include "rules/MemoryLeakRule.h"
#include "rules/UseBeforeInitRule.h"
#include "test_support.h"

// Проверяет, что параметры и присваивания считаются инициализацией переменной.
TEST_CASE("UseBeforeInitRule treats parameters and assignments as initialized")
{
    Config cfg;
    UseBeforeInitRule rule;

    auto ok = makeFileContextFromText("ok.cpp", "int id(int x){ int y; y = x; return y; }\n");
    AnalysisResult okResult;
    rule.apply(ok, cfg, okResult);
    REQUIRE(okResult.issues.empty());

    auto bad = makeFileContextFromText("bad.cpp", "int main(){ int x; int y = x; return y; }\n");
    AnalysisResult badResult;
    rule.apply(bad, cfg, badResult);
    REQUIRE(badResult.errorCount() == 1);
}

// Проверяет ошибки new/delete[] и delete без соответствующего new.
TEST_CASE("MemoryLeakRule detects array delete mismatch and delete without new")
{
    Config cfg;
    MemoryLeakRule rule;
    auto ctx = makeFileContextFromText("memory.cpp",
        "int main(){ int* a = new int[4]; delete a; int* b; delete b; return 0; }\n");
    AnalysisResult result;
    rule.apply(ctx, cfg, result);
    REQUIRE(result.warningCount() >= 2);
}
