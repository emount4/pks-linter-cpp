#include "InteractiveConfig.h"

#include <algorithm>
#include <cctype>
#include <exception>
#include <system_error>
#include <sstream>
#include <unordered_set>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

// Убирает пробельные символы по краям строки.
std::string trim(std::string s)
{
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front())) != 0) {
        s.erase(s.begin());
    }
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back())) != 0) {
        s.pop_back();
    }
    return s;
}

// Приводит строку к нижнему регистру.
std::string lower(std::string s)
{
    for (auto& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

// Разбивает строку со значениями через запятую.
std::vector<std::string> splitComma(const std::string& s)
{
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ',')) {
        item = trim(item);
        if (!item.empty()) {
            out.push_back(item);
        }
    }
    return out;
}

// Выводит приглашение и читает одну строку ответа.
std::string ask(std::istream& input, std::ostream& output, const std::string& prompt)
{
    output << prompt;
    std::string value;
    std::getline(input, value);
    return trim(value);
}

// Преобразует введенный текст в путь с поддержкой UTF-8 на Windows.
std::filesystem::path pathFromInput(const std::string& value)
{
#ifdef _WIN32
    if (value.empty()) {
        return {};
    }

    std::filesystem::path fallback;
    std::unordered_set<unsigned int> seen;
    const unsigned int codePages[] = {CP_UTF8, GetACP(), GetOEMCP()};
    for (const auto codePage : codePages) {
        if (!seen.insert(codePage).second) {
            continue;
        }
        const auto flags = codePage == CP_UTF8 ? MB_ERR_INVALID_CHARS : 0;
        int wlen = MultiByteToWideChar(codePage, flags, value.c_str(), -1, nullptr, 0);
        if (wlen <= 0) {
            continue;
        }

        std::wstring wide(static_cast<std::size_t>(wlen - 1), L'\0');
        MultiByteToWideChar(codePage, flags, value.c_str(), -1, wide.data(), wlen);
        std::filesystem::path candidate(wide);
        if (fallback.empty()) {
            fallback = candidate;
        }

        std::error_code ec;
        if (std::filesystem::exists(candidate, ec) ||
            (!candidate.parent_path().empty() && std::filesystem::exists(candidate.parent_path(), ec))) {
            return candidate;
        }
    }

    if (!fallback.empty()) {
        return fallback;
    }
#endif
    return std::filesystem::path(value);
}

// Разбирает число или возвращает значение по умолчанию.
int parseIntOrDefault(const std::string& value, int fallback, int minValue)
{
    if (value.empty()) {
        return fallback;
    }
    try {
        std::size_t consumed = 0;
        const int parsed = std::stoi(value, &consumed);
        if (consumed == value.size() && parsed >= minValue) {
            return parsed;
        }
    } catch (const std::exception&) {
    }
    return fallback;
}

// Разбирает логический ответ пользователя или возвращает значение по умолчанию.
bool parseBoolOrDefault(const std::string& value, bool fallback)
{
    const auto v = lower(value);
    if (v.empty()) {
        return fallback;
    }
    if (v == "y" || v == "yes" || v == "true" || v == "1") {
        return true;
    }
    if (v == "n" || v == "no" || v == "false" || v == "0") {
        return false;
    }
    return fallback;
}

}

// Проводит интерактивный опрос пользователя и возвращает выбранные настройки.
InteractiveSessionResult InteractiveConfig::collect(std::istream& input, std::ostream& output, Config defaults)
{
    InteractiveSessionResult result;
    result.config = std::move(defaults);

    output << "Интерактивный режим\n";
    result.projectPath = pathFromInput(ask(input, output, "Путь к проекту: "));

    const auto mode = lower(ask(input, output, "Режим (style/full) [full]: "));
    result.config.mode = mode == "style" ? AnalysisMode::Style : AnalysisMode::Full;

    result.config.indentSize = parseIntOrDefault(
        ask(input, output, "Размер отступа [" + std::to_string(result.config.indentSize) + "]: "),
        result.config.indentSize, 1);
    result.config.maxLineLength = parseIntOrDefault(
        ask(input, output, "Максимальная длина строки [" + std::to_string(result.config.maxLineLength) + "]: "),
        result.config.maxLineLength, 1);
    result.config.allowTabs = parseBoolOrDefault(
        ask(input, output, "Разрешить табуляцию? (y/n) [" + std::string(result.config.allowTabs ? "y" : "n") + "]: "),
        result.config.allowTabs);

    auto dirs = splitComma(ask(input, output, "Исключаемые директории через запятую (пусто = значения по умолчанию): "));
    if (!dirs.empty()) {
        result.config.excludedDirs.insert(result.config.excludedDirs.end(), dirs.begin(), dirs.end());
    }

    auto files = splitComma(ask(input, output, "Исключаемые файлы через запятую: "));
    if (!files.empty()) {
        result.config.excludedFiles.insert(result.config.excludedFiles.end(), files.begin(), files.end());
    }

    const bool save = parseBoolOrDefault(ask(input, output, "Сохранить конфигурацию? (y/n) [n]: "), false);
    if (save) {
        result.saveConfigPath = pathFromInput(ask(input, output, "Путь для сохранения конфигурации: "));
    }

    return result;
}
