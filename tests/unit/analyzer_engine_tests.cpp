#include <catch2/catch_test_macros.hpp>

#include "AnalyzerEngine.h"
#include "Config.h"
#include "test_support.h"
#include "scenario_helpers.h"

// Проверяет анализ проекта с явно выбранным набором правил.
TEST_CASE("AnalyzerEngine analyzes a project with selected rules only")
{
    auto root = makeTempDir("engine_selected_rules");
    writeTextFile(root / "main.cpp",
        "#define max_value 10\n"
        "int Bad_function_name(){ int BadVariable = 0; return BadVariable; }\n");

    Config cfg;
    cfg.enabledRules = {"STYLE-NAMING"};

    AnalyzerEngine engine;
    auto result = engine.analyzeProject(root, cfg);

    REQUIRE(result.filesChecked == 1);
    REQUIRE(result.errorCount() == 0);
    REQUIRE(result.warningCount() == 3);
}

// Проверяет, что движок находит нарушения пробелов при включенном правиле.
TEST_CASE("AnalyzerEngine reports spacing issues when spacing rule is enabled")
{
    auto root = makeTempDir("engine_spacing");
    writeTextFile(root / "main.cpp", "int main(){int a=1,b=2; return a+b;}\n");

    Config cfg;
    cfg.enabledRules = {"STYLE-SPACING"};

    AnalyzerEngine engine;
    auto result = engine.analyzeProject(root, cfg);

    REQUIRE(result.filesChecked == 1);
    REQUIRE(result.warningCount() == 4);
}

// Проверяет, что движок находит ошибки использования до инициализации.
TEST_CASE("AnalyzerEngine reports use-before-init errors")
{
    auto root = makeTempDir("engine_use_before");
    writeTextFile(root / "main.cpp", "int main(){ int x; return x; }\n");

    Config cfg;
    cfg.enabledRules = {"BUG-USE-BEFORE-INIT"};

    AnalyzerEngine engine;
    auto result = engine.analyzeProject(root, cfg);

    REQUIRE(result.filesChecked == 1);
    REQUIRE(result.errorCount() == 1);
    REQUIRE(result.warningCount() == 0);
}

// Проверяет события observer: начало файла, нарушение, конец файла и завершение анализа.
TEST_CASE("AnalyzerEngine notifies observers about files and issues")
{
    auto root = makeTempDir("engine_observer");
    writeTextFile(root / "a.cpp", "int a = 1;\n");
    writeTextFile(root / "b.cpp", "int main(){ int x; return x; }\n");

    Config cfg;
    cfg.enabledRules = {"BUG-USE-BEFORE-INIT"};

    AnalyzerEngine engine;
    RecordingObserver observer;
    engine.addObserver(&observer);

    auto result = engine.analyzeProject(root, cfg);

    REQUIRE(result.filesChecked == 2);
    REQUIRE(observer.starts.size() == 2);
    REQUIRE(observer.ends.size() == 2);
    REQUIRE(observer.issues.size() == 1);
    REQUIRE(observer.finishedCount == 1);
}

// Проверяет, что отключенные правила не применяются движком анализа.
TEST_CASE("AnalyzerEngine respects disabled rules")
{
    auto root = makeTempDir("engine_disabled");
    writeTextFile(root / "main.cpp", "int main(){int a=1,b=2; return a+b;}\n");

    Config cfg;
    cfg.enabledRules = {"STYLE-SPACING"};
    cfg.disabledRuleIds.insert("STYLE-SPACING");

    AnalyzerEngine engine;
    auto result = engine.analyzeProject(root, cfg);

    REQUIRE(result.filesChecked == 1);
    REQUIRE(result.issues.empty());
}
