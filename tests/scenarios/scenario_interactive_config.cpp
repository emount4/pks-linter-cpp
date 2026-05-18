#include "InteractiveConfig.h"
#include "scenario_support.h"

#include <iostream>
#include <sstream>

/**
 * Эмулирует ввод пользователя через std::istringstream и проверяет,
 * что InteractiveConfig правильно собирает параметры: путь к проекту,
 * режим анализа, настройки стиля, исключения и сохранение в файл.
 */
int main()
{
    scenarioEnableUtf8Console();
    std::istringstream input(
        "sample_project\n"
        "style\n"
        "2\n"
        "100\n"
        "n\n"
        "build,generated\n"
        "skip.cpp\n"
        "y\n"
        "saved.ini\n");
    std::ostringstream output;

    auto result = InteractiveConfig::collect(input, output);
    if (result.projectPath.generic_string() != "sample_project") {
        return 1;
    }
    if (result.config.mode != AnalysisMode::Style || result.config.indentSize != 2 ||
        result.config.maxLineLength != 100 || result.config.allowTabs) {
        return 2;
    }
    if (result.config.excludedFiles.empty() || result.config.excludedFiles.front() != "skip.cpp") {
        return 3;
    }
    if (result.saveConfigPath.generic_string() != "saved.ini") {
        return 4;
    }

    std::cout << "Сценарий интерактивной конфигурации пройден\n";
    return 0;
}
