#include "ConfigLoader.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace {

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

bool parseBool(const std::string& v, bool* out)
{
    std::string s;
    s.reserve(v.size());
    for (char c : v) {
        s.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }

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

NamingStyle parseNamingStyle(const std::string& v)
{
    std::string s;
    s.reserve(v.size());
    for (char c : v) {
        s.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }

    if (s == "lowercamel" || s == "lower_camel" || s == "camel") {
        return NamingStyle::LowerCamel;
    }
    if (s == "uppercamel" || s == "upper_camel" || s == "pascal" || s == "pascalcase") {
        return NamingStyle::UpperCamel;
    }
    if (s == "lowersnake" || s == "lower_snake" || s == "snake" || s == "snake_case") {
        return NamingStyle::LowerSnake;
    }
    if (s == "uppersnake" || s == "upper_snake" || s == "upper_snake_case" || s == "upper-snake") {
        return NamingStyle::UpperSnake;
    }
    return NamingStyle::Any;
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

} // namespace

ConfigLoadResult ConfigLoader::loadFromFile(const std::filesystem::path& path)
{
    ConfigLoadResult res;

    std::ifstream in(path);
    if (!in) {
        res.warning = "Could not open config file: " + path.string();
        return res;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        auto stripped = trim(line);
        if (stripped.empty() || stripped[0] == '#') {
            continue;
        }

        auto eq = stripped.find('=');
        if (eq == std::string::npos) {
            continue;
        }

        auto key = trim(stripped.substr(0, eq));
        auto value = trim(stripped.substr(eq + 1));

        if (key == "extensions") {
            res.config.extensions = splitCommaList(value);
            for (auto& ext : res.config.extensions) {
                if (!ext.empty() && ext.front() != '.') {
                    ext = "." + ext;
                }
            }
            continue;
        }

        if (key == "exclude.dirs") {
            auto dirs = splitCommaList(value);
            res.config.excludedDirs.insert(res.config.excludedDirs.end(), dirs.begin(), dirs.end());
            continue;
        }

        if (key == "enabled.rules" || key == "rules.enabled") {
            res.config.enabledRules = splitCommaList(value);
            continue;
        }

        bool b = false;
        if (key == "style.indentation.enabled" && parseBool(value, &b)) {
            res.config.indentationEnabled = b;
        } else if (key == "style.indentation.indent_size") {
            res.config.indentSize = std::max(1, std::stoi(value));
        } else if (key == "style.indentation.allow_tabs" && parseBool(value, &b)) {
            res.config.allowTabs = b;
        } else if (key == "style.line_length.enabled" && parseBool(value, &b)) {
            res.config.lineLengthEnabled = b;
        } else if (key == "style.line_length.max") {
            res.config.maxLineLength = std::max(20, std::stoi(value));
        } else if (key == "style.spacing.enabled" && parseBool(value, &b)) {
            res.config.spacingEnabled = b;
        } else if (key == "style.naming.enabled" && parseBool(value, &b)) {
            res.config.namingEnabled = b;
        } else if (key == "style.naming.variables") {
            res.config.variableNaming = parseNamingStyle(value);
        } else if (key == "style.naming.functions") {
            res.config.functionNaming = parseNamingStyle(value);
        } else if (key == "style.naming.constants") {
            res.config.constantNaming = parseNamingStyle(value);
        } else if (key == "bug.use_before_init.enabled" && parseBool(value, &b)) {
            res.config.useBeforeInitEnabled = b;
        } else if (key == "bug.memory_leak.enabled" && parseBool(value, &b)) {
            res.config.memoryLeakEnabled = b;
        }
    }

    return res;
}

bool ConfigLoader::saveToFile(const Config& config, const std::filesystem::path& path, std::string* error)
{
    std::ofstream out(path);
    if (!out) {
        if (error) {
            *error = "Could not write config file: " + path.string();
        }
        return false;
    }

    out << "# cpp_linter config (key=value)\n\n";
    out << "extensions=";
    for (std::size_t i = 0; i < config.extensions.size(); ++i) {
        if (i != 0) {
            out << ',';
        }
        out << config.extensions[i];
    }
    out << "\n\n";

    if (!config.excludedDirs.empty()) {
        out << "exclude.dirs=";
        for (std::size_t i = 0; i < config.excludedDirs.size(); ++i) {
            if (i != 0) {
                out << ',';
            }
            out << config.excludedDirs[i];
        }
        out << "\n\n";
    }

    if (!config.enabledRules.empty()) {
        out << "enabled.rules=";
        for (std::size_t i = 0; i < config.enabledRules.size(); ++i) {
            if (i != 0) {
                out << ',';
            }
            out << config.enabledRules[i];
        }
        out << "\n\n";
    }

    out << "style.indentation.enabled=" << (config.indentationEnabled ? "true" : "false") << "\n";
    out << "style.indentation.indent_size=" << config.indentSize << "\n";
    out << "style.indentation.allow_tabs=" << (config.allowTabs ? "true" : "false") << "\n\n";

    out << "style.line_length.enabled=" << (config.lineLengthEnabled ? "true" : "false") << "\n";
    out << "style.line_length.max=" << config.maxLineLength << "\n\n";

    out << "style.spacing.enabled=" << (config.spacingEnabled ? "true" : "false") << "\n\n";

    out << "style.naming.enabled=" << (config.namingEnabled ? "true" : "false") << "\n";
    out << "style.naming.variables=lowerCamel\n";
    out << "style.naming.functions=lowerCamel\n";
    out << "style.naming.constants=UPPER_SNAKE\n\n";

    out << "bug.use_before_init.enabled=" << (config.useBeforeInitEnabled ? "true" : "false") << "\n";
    out << "bug.memory_leak.enabled=" << (config.memoryLeakEnabled ? "true" : "false") << "\n";

    return true;
}
