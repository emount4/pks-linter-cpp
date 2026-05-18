#include "AnalyzerEngine.h"
#include "Config.h"
#include "scenario_support.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

/**
 * Генерирует временный файл с 10 000 строк кода и запускает анализ.
 * Проверяет, что анализатор не падает на больших объёмах и укладывается
 * в лимит времени (10 секунд). Тест на производительность и стабильность.
 */

int main()
{
    scenarioEnableUtf8Console();

    auto root = std::filesystem::temp_directory_path() / "cpp_linter_large_project_scenario";
    std::error_code ec;
    std::filesystem::remove_all(root, ec);
    std::filesystem::create_directories(root, ec);

    {
        std::ofstream out(root / "large.cpp", std::ios::binary);
        out << "int main(){\n";
        for (int i = 0; i < 9998; ++i) {
            out << "    int value_" << i << " = " << i << ";\n";
        }
        out << "    return 0;\n}\n";
    }

    Config config;
    config.mode = AnalysisMode::Style;
    config.enabledRules = {"STYLE-LINE-LENGTH", "STYLE-SPACING"};
    AnalyzerEngine engine;

    const auto start = std::chrono::steady_clock::now();
    auto result = engine.analyzeProject(root, config);
    const auto elapsed = std::chrono::steady_clock::now() - start;

    if (result.filesChecked != 1) {
        return 1;
    }
    if (elapsed > std::chrono::seconds(10)) {
        std::cerr << "Сценарий большого проекта выполнялся слишком долго\n";
        return 2;
    }

    std::cout << "Сценарий большого проекта пройден, нарушений: " << result.issues.size() << '\n';
    return 0;
}
