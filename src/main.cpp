#include "AnalyzerEngine.h"
#include "ConfigLoader.h"
#include "ConfigManager.h"
#include "ConsoleProgressObserver.h"
#include "InteractiveConfig.h"
#include "Reporter.h"
#include "ReporterObserver.h"
#include "Rule.h"
#include "RuleFactory.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

struct CliOptions {
    std::filesystem::path project;
    std::filesystem::path configPath;
    std::filesystem::path saveConfigPath;
    std::vector<std::string> excludeDirs;
    std::vector<std::string> excludeFiles;
    std::vector<std::string> disableRules;
    bool interactive{false};
    bool help{false};
    bool showProgress{false};
    bool modeProvided{false};
    AnalysisMode mode{AnalysisMode::Full};
};

std::filesystem::path pathFromUserInput(const std::string& s)
{
#ifdef _WIN32
    if (s.empty()) {
        return {};
    }
    int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(), -1, nullptr, 0);
    if (wlen > 0) {
        std::wstring ws(static_cast<std::size_t>(wlen - 1), L'\0');
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(), -1, ws.data(), wlen);
        return std::filesystem::path(ws);
    }
#endif
    return std::filesystem::path(s);
}

std::string canonicalRuleId(const std::string& id)
{
    if (auto rule = RuleFactory::createById(id)) {
        return rule->id();
    }
    return id;
}

std::vector<std::string> rulesForMode(AnalysisMode mode)
{
    if (mode == AnalysisMode::Style) {
        return {"STYLE-INDENTATION", "STYLE-SPACING", "STYLE-LINE-LENGTH", "STYLE-NAMING"};
    }
    return {"STYLE-INDENTATION", "STYLE-SPACING", "STYLE-LINE-LENGTH", "STYLE-NAMING",
        "BUG-USE-BEFORE-INIT", "BUG-MEMORY-LEAK"};
}

void applyMode(Config& config, bool force)
{
    if (force || config.enabledRules.empty()) {
        config.enabledRules = rulesForMode(config.mode);
    }
}

void printHelp(std::ostream& os)
{
    os << "cpp_linter - учебный статический анализатор для подмножества C++\n\n";
    os << "Использование:\n";
    os << "  cpp_linter --project <path> [options]\n";
    os << "  cpp_linter --interactive\n\n";
    os << "Параметры:\n";
    os << "  --help                    Показать эту справку\n";
    os << "  --project <path>          Директория проекта для анализа\n";
    os << "  --config <path>           Загрузить INI-конфигурацию\n";
    os << "  --mode <style|full>       Выбрать режим: только стиль или полная проверка\n";
    os << "  --disable <rule-id>       Отключить правило после применения режима/config\n";
    os << "  --exclude-dir <name>      Исключить директорию по имени, относительному или абсолютному пути\n";
    os << "  --exclude-file <name>     Исключить файл по имени или относительному пути\n";
    os << "  --interactive             Запросить настройки через консоль\n";
    os << "  --save-config <path>      Сохранить итоговую конфигурацию\n";
    os << "  --show-progress           Показать ход проверки по файлам\n\n";
    os << "Примеры:\n";
    os << "  cpp_linter --project .\\sample_project --mode style\n";
    os << "  cpp_linter --project .\\sample_project --config .\\config.example.ini\n";
    os << "  cpp_linter --project .\\sample_project --disable STYLE-SPACING\n";
}

CliOptions parseArgs(int argc, char** argv)
{
    CliOptions opt;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        auto requireValue = [&](const std::string& name) -> const char* {
            if (i + 1 >= argc) {
                std::cerr << "[ОШИБКА] Не указано значение для " << name << '\n';
                return "";
            }
            return argv[++i];
        };

        if (arg == "--help" || arg == "-h") {
            opt.help = true;
        } else if (arg == "--interactive") {
            opt.interactive = true;
        } else if (arg == "--project") {
            opt.project = pathFromUserInput(requireValue(arg));
        } else if (arg == "--config") {
            opt.configPath = pathFromUserInput(requireValue(arg));
        } else if (arg == "--save-config") {
            opt.saveConfigPath = pathFromUserInput(requireValue(arg));
        } else if (arg == "--mode") {
            const std::string mode = requireValue(arg);
            opt.modeProvided = true;
            opt.mode = mode == "style" ? AnalysisMode::Style : AnalysisMode::Full;
        } else if (arg == "--disable") {
            opt.disableRules.push_back(requireValue(arg));
        } else if (arg == "--exclude-dir" || arg == "--exclude") {
            opt.excludeDirs.push_back(requireValue(arg));
        } else if (arg == "--exclude-file") {
            opt.excludeFiles.push_back(requireValue(arg));
        } else if (arg == "--show-progress") {
            opt.showProgress = true;
        } else {
            std::cerr << "[ПРЕДУПРЕЖДЕНИЕ] Неизвестный параметр проигнорирован: " << arg << '\n';
        }
    }
    return opt;
}

} // пространство имен

int main(int argc, char** argv)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    auto options = parseArgs(argc, argv);
    if (options.help) {
        printHelp(std::cout);
        return 0;
    }

    Config config;
    if (!options.configPath.empty()) {
        const auto loaded = ConfigLoader::loadFromFile(options.configPath);
        config = loaded.config;
        if (loaded.warning) {
            std::cerr << "[ПРЕДУПРЕЖДЕНИЕ] " << *loaded.warning << '\n';
        }
    }

    if (options.interactive || argc == 1) {
        auto collected = InteractiveConfig::collect(std::cin, std::cout, config);
        config = collected.config;
        if (!collected.projectPath.empty()) {
            options.project = collected.projectPath;
        }
        if (!collected.saveConfigPath.empty()) {
            options.saveConfigPath = collected.saveConfigPath;
        }
        options.interactive = true;
    }

    if (options.modeProvided) {
        config.mode = options.mode;
    }
    applyMode(config, options.modeProvided);

    if (options.showProgress) {
        config.showProgress = true;
    }
    config.excludedDirs.insert(config.excludedDirs.end(), options.excludeDirs.begin(), options.excludeDirs.end());
    config.excludedFiles.insert(config.excludedFiles.end(), options.excludeFiles.begin(), options.excludeFiles.end());
    for (const auto& ruleId : options.disableRules) {
        config.disabledRuleIds.insert(canonicalRuleId(ruleId));
    }

    ConfigManager::instance().set(config);

    if (options.project.empty()) {
        std::cerr << "[ОШИБКА] Требуется --project, если не используется --interactive.\n";
        printHelp(std::cerr);
        return 2;
    }

    if (!options.saveConfigPath.empty()) {
        std::string error;
        if (!ConfigManager::instance().saveToFile(options.saveConfigPath, &error)) {
            std::cerr << "[ПРЕДУПРЕЖДЕНИЕ] " << error << '\n';
        }
    }

    AnalyzerEngine engine;
    ConsoleProgressObserver progress(std::cout);
    ReporterObserver reporter(std::cout);
    if (config.showProgress) {
        engine.addObserver(&progress);
    }
    engine.addObserver(&reporter);

    (void)engine.analyzeProject(options.project, ConfigManager::instance().get());
    return 0;
}
