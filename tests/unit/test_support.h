#pragma once

#include "AnalysisResult.h"
#include "Config.h"
#include "FileContext.h"
#include "Issue.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

inline std::filesystem::path makeTempDir(const std::string& prefix)
{
    static std::atomic<unsigned long long> counter{0};
    auto base = std::filesystem::temp_directory_path();
    auto stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto dir = base / (prefix + "_" + std::to_string(stamp) + "_" + std::to_string(counter.fetch_add(1)));
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
    std::filesystem::create_directories(dir, ec);
    return dir;
}

inline void writeTextFile(const std::filesystem::path& path, const std::string& content)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out << content;
}

inline std::vector<std::string> splitLines(const std::string& content)
{
    std::vector<std::string> lines;
    std::string current;
    for (char ch : content) {
        if (ch == '\n') {
            if (!current.empty() && current.back() == '\r') {
                current.pop_back();
            }
            lines.push_back(current);
            current.clear();
        } else {
            current.push_back(ch);
        }
    }
    if (!current.empty()) {
        if (!current.empty() && current.back() == '\r') {
            current.pop_back();
        }
        lines.push_back(current);
    }
    return lines;
}

inline FileContext makeFileContextFromText(std::filesystem::path path, const std::string& content)
{
    Tokenizer tokenizer;
    FileContext ctx;
    ctx.path = std::move(path);
    ctx.lines = splitLines(content);
    ctx.tokens = tokenizer.tokenize(content).tokens;
    return ctx;
}

inline AnalysisResult makeResult(std::initializer_list<Issue> issues, std::size_t filesChecked = 0)
{
    AnalysisResult result;
    result.filesChecked = filesChecked;
    for (const auto& issue : issues) {
        result.addIssue(issue);
    }
    return result;
}

inline FileContext makeFileContext(std::filesystem::path path, std::vector<std::string> lines, std::vector<Token> tokens = {})
{
    FileContext ctx;
    ctx.path = std::move(path);
    ctx.lines = std::move(lines);
    ctx.tokens = std::move(tokens);
    return ctx;
}
