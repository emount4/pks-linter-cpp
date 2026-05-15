#include "AnalyzerEngine.h"
#include "ConfigManager.h"
#include "ConfigLoader.h"
#include "Reporter.h"
#include "ReporterObserver.h"
#include "RuleFactory.h"

#include <filesystem>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <tlhelp32.h>
#endif

namespace {

struct CliOptions {
    std::filesystem::path project;
    std::filesystem::path configPath;
    std::filesystem::path saveConfigPath;
    std::vector<std::string> excludes;
    std::vector<std::string> disableRules;
    bool interactive{false};
    bool help{false};
};

void printHelp(std::ostream& os)
{
    os << "cpp_linter — учебный статический анализатор (C++ subset)\n\n";
    os << "Usage:\n";
    os << "  cpp_linter --project <path> [--config <file>] [--exclude <dir>]... [--disable <RULE_ID>]...\n";
    os << "  cpp_linter --interactive\n\n";
    os << "Options:\n";
    os << "  --project <path>     Path to project folder\n";
    os << "  --config <file>      Config file (key=value)\n";
    os << "  --save-config <file> Save resulting config (interactive or CLI)\n";
    os << "  --exclude <dir>      Exclude directory by name (e.g. build, .git)\n";
    os << "  --disable <RULE_ID>  Disable rule by id (e.g. STYLE-SPACING)\n";
    os << "  --interactive        Ask parameters via console\n";
    os << "  --help               Show this help\n\n";
    os << "Rule IDs:\n";
    const auto& ids = RuleFactory::allRuleIds();
    if (ids.size() >= 4) {
        os << "  " << ids[0] << ", " << ids[1] << ", " << ids[2] << ", " << ids[3] << "\n";
        if (ids.size() > 4) {
            os << "  ";
            for (std::size_t i = 4; i < ids.size(); ++i) {
                if (i != 4) {
                    os << ", ";
                }
                os << ids[i];
            }
            os << "\n";
        }
    } else {
        for (const auto& id : ids) {
            os << "  " << id << "\n";
        }
    }
}

std::string promptLine(const std::string& label)
{
    std::cout << label;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

int promptChoice(const std::string& title, const std::vector<std::string>& options, int defaultChoice)
{
    if (options.empty()) {
        return defaultChoice;
    }

    for (;;) {
        std::cout << title << "\n";
        for (std::size_t i = 0; i < options.size(); ++i) {
            std::cout << "  " << (i + 1) << ") " << options[i] << "\n";
        }
        std::cout << "Choice";
        if (defaultChoice >= 1 && defaultChoice <= static_cast<int>(options.size())) {
            std::cout << " [" << defaultChoice << "]";
        }
        std::cout << ": ";

        std::string s;
        std::getline(std::cin, s);
        if (s.empty() && defaultChoice >= 1 && defaultChoice <= static_cast<int>(options.size())) {
            return defaultChoice;
        }

        try {
            int v = std::stoi(s);
            if (v >= 1 && v <= static_cast<int>(options.size())) {
                return v;
            }
        } catch (...) {
        }

        std::cout << "Invalid choice. Please enter a number from 1 to " << options.size() << ".\n\n";
    }
}

bool promptBoolChoice(const std::string& title, bool defaultValue)
{
    int def = defaultValue ? 1 : 2;
    int c = promptChoice(title, {"Yes", "No"}, def);
    return c == 1;
}

int promptInt(const std::string& label, int defaultValue, int minValue)
{
    for (;;) {
        std::cout << label << " [" << defaultValue << "]: ";
        std::string s;
        std::getline(std::cin, s);
        if (s.empty()) {
            return defaultValue;
        }

        try {
            int v = std::stoi(s);
            if (v >= minValue) {
                return v;
            }
        } catch (...) {
        }
        std::cout << "Invalid number (min " << minValue << ").\n";
    }
}

NamingStyle promptNamingStyle(const std::string& title, NamingStyle current)
{
    int def = 1;
    if (current == NamingStyle::LowerCamel) {
        def = 2;
    } else if (current == NamingStyle::UpperCamel) {
        def = 3;
    } else if (current == NamingStyle::LowerSnake) {
        def = 4;
    } else if (current == NamingStyle::UpperSnake) {
        def = 5;
    }

    int c = promptChoice(title,
        {"Any (do not check)", "lowerCamelCase", "UpperCamelCase", "snake_case", "UPPER_SNAKE_CASE"},
        def);
    if (c == 2) {
        return NamingStyle::LowerCamel;
    }
    if (c == 3) {
        return NamingStyle::UpperCamel;
    }
    if (c == 4) {
        return NamingStyle::LowerSnake;
    }
    if (c == 5) {
        return NamingStyle::UpperSnake;
    }
    return NamingStyle::Any;
}

std::vector<std::string> splitComma(const std::string& s)
{
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ',')) {
        // trim
        while (!item.empty() && std::isspace(static_cast<unsigned char>(item.front())) != 0) {
            item.erase(item.begin());
        }
        while (!item.empty() && std::isspace(static_cast<unsigned char>(item.back())) != 0) {
            item.pop_back();
        }
        if (!item.empty()) {
            out.push_back(item);
        }
    }
    return out;
}

std::filesystem::path pathFromUserInput(const std::string& s)
{
#ifdef _WIN32
    if (s.empty()) {
        return std::filesystem::path();
    }

    auto decode = [&](UINT codePage, DWORD flags) -> std::wstring {
        int wlen = MultiByteToWideChar(codePage, flags, s.c_str(), -1, nullptr, 0);
        if (wlen <= 0) {
            return std::wstring();
        }

        std::wstring ws(static_cast<std::size_t>(wlen - 1), L'\0');
        int converted = MultiByteToWideChar(codePage, flags, s.c_str(), -1, ws.data(), wlen);
        if (converted <= 0) {
            return std::wstring();
        }
        return ws;
    };

    // Prefer UTF-8, but gracefully accept current ANSI/OEM console encodings.
    std::wstring ws = decode(CP_UTF8, MB_ERR_INVALID_CHARS);
    if (ws.empty()) {
        ws = decode(CP_ACP, 0);
    }
    if (ws.empty()) {
        ws = decode(CP_OEMCP, 0);
    }

    if (!ws.empty()) {
        return std::filesystem::path(ws);
    }
    return std::filesystem::path(s);
#else
    return std::filesystem::path(s);
#endif
}

#ifdef _WIN32
bool launchedFromExplorer()
{
    const DWORD currentPid = GetCurrentProcessId();
    DWORD parentPid = 0;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(entry);
    if (Process32FirstW(snapshot, &entry) != FALSE) {
        do {
            if (entry.th32ProcessID == currentPid) {
                parentPid = entry.th32ParentProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &entry) != FALSE);
    }

    if (parentPid == 0) {
        CloseHandle(snapshot);
        return false;
    }

    std::wstring parentName;
    entry.dwSize = sizeof(entry);
    if (Process32FirstW(snapshot, &entry) != FALSE) {
        do {
            if (entry.th32ProcessID == parentPid) {
                parentName = entry.szExeFile;
                break;
            }
        } while (Process32NextW(snapshot, &entry) != FALSE);
    }

    CloseHandle(snapshot);
    return _wcsicmp(parentName.c_str(), L"explorer.exe") == 0;
}

void pauseBeforeExitIfNeeded()
{
    if (!launchedFromExplorer()) {
        return;
    }

    std::cout << "\nPress Enter to close this window...";
    std::cout.flush();
    std::string dummy;
    std::getline(std::cin, dummy);
}
#endif

CliOptions parseArgs(int argc, char** argv)
{
    CliOptions opt;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--help" || a == "-h") {
            opt.help = true;
            continue;
        }
        if (a == "--interactive") {
            opt.interactive = true;
            continue;
        }
        if (a == "--project" && i + 1 < argc) {
            opt.project = pathFromUserInput(argv[++i]);
            continue;
        }
        if (a == "--config" && i + 1 < argc) {
            opt.configPath = pathFromUserInput(argv[++i]);
            continue;
        }
        if (a == "--save-config" && i + 1 < argc) {
            opt.saveConfigPath = pathFromUserInput(argv[++i]);
            continue;
        }
        if (a == "--exclude" && i + 1 < argc) {
            opt.excludes.push_back(argv[++i]);
            continue;
        }
        if (a == "--disable" && i + 1 < argc) {
            opt.disableRules.push_back(argv[++i]);
            continue;
        }
    }
    return opt;
}

} // namespace

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

    const bool needInteractive = options.interactive || options.project.empty();

    if (!needInteractive) {
        if (!options.configPath.empty()) {
            if (auto warning = ConfigManager::instance().loadFromFile(options.configPath)) {
                std::cerr << "[WARNING] " << *warning << "\n";
            }
            config = ConfigManager::instance().get();
        }
    } else {
        std::cout << "Interactive mode\n\n";

        auto p = promptLine("Project path: ");
        if (!p.empty()) {
            options.project = pathFromUserInput(p);
        }

        const bool hasCliConfig = !options.configPath.empty();
        int defaultUseConfig = hasCliConfig ? 1 : 2;
        bool useConfigFile = (promptChoice("Use configuration file?", {"Yes", "No"}, defaultUseConfig) == 1);

        if (useConfigFile) {
            if (options.configPath.empty()) {
                auto cfg = promptLine("Config file path: ");
                if (!cfg.empty()) {
                    options.configPath = pathFromUserInput(cfg);
                }
            }

            if (!options.configPath.empty()) {
                if (auto warning = ConfigManager::instance().loadFromFile(options.configPath)) {
                    std::cerr << "[WARNING] " << *warning << "\n";
                }
                config = ConfigManager::instance().get();
            } else {
                std::cerr << "[WARNING] No config path provided. Using defaults.\n";
            }
        } else {
            // Prompt all parameters (numbered choices where applicable)
            if (promptChoice("Extensions:", {"Use defaults (.cpp,.h,.hpp)", "Enter custom list"}, 1) == 2) {
                auto s = promptLine("Extensions (comma-separated, e.g. .cpp,.h,.hpp): ");
                auto exts = splitComma(s);
                if (!exts.empty()) {
                    config.extensions = exts;
                }
            }

            {
                auto s = promptLine("Excluded dirs (comma-separated, empty for none): ");
                auto dirs = splitComma(s);
                config.excludedDirs.insert(config.excludedDirs.end(), dirs.begin(), dirs.end());
            }

            config.indentationEnabled = promptBoolChoice("Enable indentation rule?", config.indentationEnabled);
            if (config.indentationEnabled) {
                config.indentSize = promptInt("Indent size", config.indentSize, 1);
                config.allowTabs = promptBoolChoice("Allow tabs?", config.allowTabs);
            }

            config.lineLengthEnabled = promptBoolChoice("Enable max line length rule?", config.lineLengthEnabled);
            if (config.lineLengthEnabled) {
                config.maxLineLength = promptInt("Max line length", config.maxLineLength, 20);
            }

            config.spacingEnabled = promptBoolChoice("Enable spacing rule?", config.spacingEnabled);

            config.namingEnabled = promptBoolChoice("Enable naming rule?", config.namingEnabled);
            if (config.namingEnabled) {
                config.variableNaming = promptNamingStyle("Variable naming style:", config.variableNaming);
                config.functionNaming = promptNamingStyle("Function naming style:", config.functionNaming);
                config.constantNaming = promptNamingStyle("Constants/macros naming style:", config.constantNaming);
            }

            config.useBeforeInitEnabled = promptBoolChoice("Enable use-before-init rule?", config.useBeforeInitEnabled);
            config.memoryLeakEnabled = promptBoolChoice("Enable memory-leak rule?", config.memoryLeakEnabled);
        }

        if (options.saveConfigPath.empty() &&
            (promptChoice("Save resulting config to file?", {"Yes", "No"}, 2) == 1)) {
            auto s = promptLine("Save config path (e.g. ./cpp_linter.ini): ");
            if (!s.empty()) {
                options.saveConfigPath = pathFromUserInput(s);
            }
        }
    }

    // Apply CLI overrides after interactive/config selection
    for (const auto& ex : options.excludes) {
        config.excludedDirs.push_back(ex);
    }
    for (const auto& rid : options.disableRules) {
        config.disabledRuleIds.insert(rid);
    }

    if (config.enabledRules.empty()) {
        config.enabledRules = RuleFactory::enabledRuleIdsFromFlags(config);
    }

    ConfigManager::instance().set(config);

    if (options.project.empty()) {
        std::cerr << "[ERROR] --project is required (or use --interactive).\n";
        printHelp(std::cerr);
        return 2;
    }

    if (!options.saveConfigPath.empty()) {
        std::string err;
        if (!ConfigLoader::saveToFile(config, options.saveConfigPath, &err)) {
            std::cerr << "[WARNING] " << err << "\n";
        }
    }

    AnalyzerEngine engine;
    ReporterObserver reporterObserver(std::cout);
    engine.addObserver(&reporterObserver);
    auto result = engine.analyzeProject(options.project, ConfigManager::instance().get());

#ifdef _WIN32
    pauseBeforeExitIfNeeded();
#endif

    return result.errorCount() > 0 ? 1 : 0;
}
