#include "ConfigLoader.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

namespace {

constexpr const char* kStyleIndent = "STYLE-INDENTATION";
constexpr const char* kStyleSpacing = "STYLE-SPACING";
constexpr const char* kStyleLineLength = "STYLE-LINE-LENGTH";
constexpr const char* kStyleNaming = "STYLE-NAMING";
constexpr const char* kBugUseBeforeInit = "BUG-USE-BEFORE-INIT";
constexpr const char* kBugMemoryLeak = "BUG-MEMORY-LEAK";

std::string trim(std::string s)
{
    auto isSpace = [](unsigned char ch) { return std::isspace(ch) != 0; };
    while (!s.empty() && isSpace(static_cast<unsigned char>(s.front()))) {
        s.erase(s.begin());
    }
    while (!s.empty() && isSpace(static_cast<unsigned char>(s.back()))) {
        s.pop_back();
    }
    return s;
}

std::string lower(std::string s)
{
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

std::string upper(std::string s)
{
    for (char& c : s) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return s;
}

std::string canonicalRuleId(std::string id)
{
    id = trim(std::move(id));
    const auto l = lower(id);
    if (l == "style-indent" || l == "style-indentation" || l == "indentation") {
        return kStyleIndent;
    }
    if (l == "style-spacing" || l == "spacing") {
        return kStyleSpacing;
    }
    if (l == "style-line-length" || l == "line-length" || l == "line_length" || l == "linelength") {
        return kStyleLineLength;
    }
    if (l == "style-naming" || l == "naming") {
        return kStyleNaming;
    }
    if (l == "bug-use-before-init" || l == "use-before-init" || l == "use_before_init" || l == "usebeforeinit") {
        return kBugUseBeforeInit;
    }
    if (l == "bug-memory-leak" || l == "memory-leak" || l == "memory_leak" || l == "memoryleak") {
        return kBugMemoryLeak;
    }
    return upper(id);
}

const std::unordered_set<std::string>& knownRules()
{
    static const std::unordered_set<std::string> ids{
        kStyleIndent, kStyleSpacing, kStyleLineLength, kStyleNaming, kBugUseBeforeInit, kBugMemoryLeak};
    return ids;
}

bool parseBool(const std::string& v, bool* out)
{
    const auto s = lower(trim(v));
    if (s == "true" || s == "1" || s == "yes" || s == "on") {
        *out = true;
        return true;
    }
    if (s == "false" || s == "0" || s == "no" || s == "off") {
        *out = false;
        return true;
    }
    return false;
}

int parseIntOrDefault(const std::string& value, int fallback, int minValue, const std::string& key,
    std::vector<std::string>& diagnostics)
{
    try {
        std::size_t consumed = 0;
        const int parsed = std::stoi(value, &consumed);
        if (consumed != trim(value).size() || parsed < minValue) {
            diagnostics.push_back("Некорректное числовое значение для '" + key + "', используется значение по умолчанию " + std::to_string(fallback));
            return fallback;
        }
        return parsed;
    } catch (const std::exception&) {
        diagnostics.push_back("Некорректное числовое значение для '" + key + "', используется значение по умолчанию " + std::to_string(fallback));
        return fallback;
    }
}

NamingStyle parseNamingStyle(const std::string& v)
{
    const auto s = lower(trim(v));
    if (s == "lowercamel" || s == "lower_camel" || s == "camel" || s == "camel_case" || s == "camelcase") {
        return NamingStyle::LowerCamel;
    }
    if (s == "uppercamel" || s == "upper_camel" || s == "pascal" || s == "pascalcase") {
        return NamingStyle::UpperCamel;
    }
    if (s == "lowersnake" || s == "lower_snake" || s == "snake" || s == "snake_case") {
        return NamingStyle::LowerSnake;
    }
    if (s == "uppersnake" || s == "upper_snake" || s == "upper_case" || s == "upper-snake" ||
        s == "upper_snake_case") {
        return NamingStyle::UpperSnake;
    }
    return NamingStyle::Any;
}

std::string namingStyleToConfig(NamingStyle style)
{
    switch (style) {
    case NamingStyle::LowerCamel:
        return "camel_case";
    case NamingStyle::UpperCamel:
        return "PascalCase";
    case NamingStyle::LowerSnake:
        return "snake_case";
    case NamingStyle::UpperSnake:
        return "upper_case";
    case NamingStyle::Any:
        return "any";
    }
    return "any";
}

std::vector<std::string> splitCommaList(const std::string& v)
{
    std::vector<std::string> parts;
    std::stringstream ss(v);
    std::string item;
    while (std::getline(ss, item, ',')) {
        item = trim(item);
        if (!item.empty()) {
            parts.push_back(item);
        }
    }
    return parts;
}

std::vector<std::string> normalizeRuleList(const std::string& value, std::vector<std::string>& diagnostics)
{
    std::vector<std::string> out;
    for (auto id : splitCommaList(value)) {
        id = canonicalRuleId(std::move(id));
        if (knownRules().find(id) == knownRules().end()) {
            diagnostics.push_back("Неизвестное правило '" + id + "' проигнорировано");
            continue;
        }
        out.push_back(std::move(id));
    }
    return out;
}

std::string join(const std::vector<std::string>& values)
{
    std::ostringstream out;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i != 0) {
            out << ',';
        }
        out << values[i];
    }
    return out.str();
}

std::string modeToString(AnalysisMode mode)
{
    return mode == AnalysisMode::Style ? "style" : "full";
}

} // пространство имен

ConfigLoadResult ConfigLoader::loadFromFile(const std::filesystem::path& path)
{
    ConfigLoadResult res;

    std::ifstream in(path);
    if (!in) {
        res.warning = "Не удалось открыть файл конфигурации: " + path.string();
        res.diagnostics.push_back(*res.warning);
        return res;
    }

    std::string section;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        auto stripped = trim(line);
        if (stripped.empty() || stripped[0] == '#' || stripped[0] == ';') {
            continue;
        }
        if (stripped.front() == '[' && stripped.back() == ']') {
            section = lower(trim(stripped.substr(1, stripped.size() - 2)));
            continue;
        }

        const auto eq = stripped.find('=');
        if (eq == std::string::npos) {
            continue;
        }

        auto key = trim(stripped.substr(0, eq));
        auto value = trim(stripped.substr(eq + 1));
        const auto fullKey = section.empty() ? key : section + "." + key;
        bool b = false;

        if (fullKey == "general.mode") {
            const auto m = lower(value);
            if (m == "style") {
                res.config.mode = AnalysisMode::Style;
            } else if (m == "full") {
                res.config.mode = AnalysisMode::Full;
            } else {
                res.diagnostics.push_back("Неизвестный режим '" + value + "', используется full");
                res.config.mode = AnalysisMode::Full;
            }
        } else if (fullKey == "general.show_progress") {
            if (parseBool(value, &b)) {
                res.config.showProgress = b;
            }
        } else if (fullKey == "extensions" || fullKey == "general.extensions") {
            res.config.extensions = splitCommaList(value);
            for (auto& ext : res.config.extensions) {
                if (!ext.empty() && ext.front() != '.') {
                    ext = "." + ext;
                }
            }
        } else if (fullKey == "exclude.dirs" || fullKey == "exclude.dirs") {
            res.config.excludedDirs = splitCommaList(value);
        } else if (fullKey == "exclude.files") {
            res.config.excludedFiles = splitCommaList(value);
        } else if (fullKey == "rules.enabled" || fullKey == "enabled.rules") {
            res.config.enabledRules = normalizeRuleList(value, res.diagnostics);
        } else if (fullKey == "rules.disabled") {
            for (auto id : normalizeRuleList(value, res.diagnostics)) {
                res.config.disabledRuleIds.insert(std::move(id));
            }
        } else if (fullKey == "style.indent_size" || fullKey == "style.indentation.indent_size") {
            res.config.indentSize = parseIntOrDefault(value, Config{}.indentSize, 1, fullKey, res.diagnostics);
        } else if (fullKey == "style.allow_tabs" || fullKey == "style.indentation.allow_tabs") {
            if (parseBool(value, &b)) {
                res.config.allowTabs = b;
            }
        } else if (fullKey == "style.max_line_length" || fullKey == "style.line_length.max") {
            res.config.maxLineLength = parseIntOrDefault(value, Config{}.maxLineLength, 1, fullKey, res.diagnostics);
        } else if (fullKey == "style.variable_case" || fullKey == "style.naming.variables") {
            res.config.variableNaming = parseNamingStyle(value);
        } else if (fullKey == "style.function_case" || fullKey == "style.naming.functions") {
            res.config.functionNaming = parseNamingStyle(value);
        } else if (fullKey == "style.constant_case" || fullKey == "style.naming.constants") {
            res.config.constantNaming = parseNamingStyle(value);
        } else if (fullKey == "style.indentation.enabled" && parseBool(value, &b)) {
            res.config.indentationEnabled = b;
        } else if (fullKey == "style.line_length.enabled" && parseBool(value, &b)) {
            res.config.lineLengthEnabled = b;
        } else if (fullKey == "style.spacing.enabled" && parseBool(value, &b)) {
            res.config.spacingEnabled = b;
        } else if (fullKey == "style.naming.enabled" && parseBool(value, &b)) {
            res.config.namingEnabled = b;
        } else if (fullKey == "bug.use_before_init.enabled" && parseBool(value, &b)) {
            res.config.useBeforeInitEnabled = b;
        } else if (fullKey == "bug.memory_leak.enabled" && parseBool(value, &b)) {
            res.config.memoryLeakEnabled = b;
        } else if (section == "severity") {
            auto id = canonicalRuleId(key);
            if (knownRules().find(id) == knownRules().end()) {
                res.diagnostics.push_back("Неизвестное правило в секции severity '" + key + "' проигнорировано");
                continue;
            }
            const auto sev = lower(value);
            if (sev == "error" || sev == "warning") {
                res.config.severityByRule[id] = sev;
            } else {
                res.config.severityByRule[id] = "warning";
                res.diagnostics.push_back("Неизвестный уровень серьезности '" + value + "' для " + id + ", используется warning");
            }
        }
    }

    for (const auto& diagnostic : res.diagnostics) {
        if (diagnostic != res.warning.value_or("")) {
            std::cerr << "[ПРЕДУПРЕЖДЕНИЕ] " << diagnostic << '\n';
        }
    }

    return res;
}

bool ConfigLoader::saveToFile(const Config& config, const std::filesystem::path& path, std::string* error)
{
    std::ofstream out(path);
    if (!out) {
        if (error) {
            *error = "Не удалось записать файл конфигурации: " + path.string();
        }
        return false;
    }

    out << "[general]\n";
    out << "mode=" << modeToString(config.mode) << "\n";
    out << "show_progress=" << (config.showProgress ? "true" : "false") << "\n\n";

    out << "[style]\n";
    out << "indentation.enabled=" << (config.indentationEnabled ? "true" : "false") << "\n";
    out << "indent_size=" << config.indentSize << "\n";
    out << "allow_tabs=" << (config.allowTabs ? "true" : "false") << "\n";
    out << "line_length.enabled=" << (config.lineLengthEnabled ? "true" : "false") << "\n";
    out << "max_line_length=" << config.maxLineLength << "\n";
    out << "spacing.enabled=" << (config.spacingEnabled ? "true" : "false") << "\n";
    out << "naming.enabled=" << (config.namingEnabled ? "true" : "false") << "\n";
    out << "variable_case=" << namingStyleToConfig(config.variableNaming) << "\n";
    out << "function_case=" << namingStyleToConfig(config.functionNaming) << "\n";
    out << "constant_case=" << namingStyleToConfig(config.constantNaming) << "\n\n";

    out << "[bug]\n";
    out << "use_before_init.enabled=" << (config.useBeforeInitEnabled ? "true" : "false") << "\n";
    out << "memory_leak.enabled=" << (config.memoryLeakEnabled ? "true" : "false") << "\n\n";

    out << "[rules]\n";
    out << "enabled=" << join(config.enabledRules) << "\n";
    std::vector<std::string> disabled(config.disabledRuleIds.begin(), config.disabledRuleIds.end());
    std::sort(disabled.begin(), disabled.end());
    out << "disabled=" << join(disabled) << "\n\n";

    out << "[severity]\n";
    for (const auto& id : {kStyleIndent, kStyleSpacing, kStyleLineLength, kStyleNaming, kBugUseBeforeInit, kBugMemoryLeak}) {
        auto it = config.severityByRule.find(id);
        if (it != config.severityByRule.end()) {
            out << id << '=' << it->second << "\n";
        } else if (std::string(id) == kBugUseBeforeInit) {
            out << id << "=error\n";
        } else {
            out << id << "=warning\n";
        }
    }

    out << "\n[exclude]\n";
    out << "dirs=" << join(config.excludedDirs) << "\n";
    out << "files=" << join(config.excludedFiles) << "\n";

    return true;
}
