#include "AnalyzerEngine.h"

#include "FileContext.h"
#include "FileScanner.h"
#include "IObserver.h"
#include "IssueEvent.h"
#include "Rule.h"
#include "RuleFactory.h"
#include "Tokenizer.h"

#include <fstream>
#include <sstream>

namespace {

// Разбивает текст файла на строки без символов перевода строки.
std::vector<std::string> splitLines(const std::string& content)
{
    std::vector<std::string> lines;
    std::stringstream ss(content);
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
    if (content.empty()) {
        return lines;
    }
    if (!content.empty() && content.back() == '\n') {
        return lines;
    }
    return lines;
}

}

// Подписывает observer на события анализа.
void AnalyzerEngine::addObserver(IObserver* obs)
{
    observers_.push_back(obs);
}

// Рассылает событие начала анализа файла.
void AnalyzerEngine::notifyFileStart(const std::string& path) const
{
    for (auto* obs : observers_) {
        if (obs) {
            obs->onFileStart(path);
        }
    }
}

// Рассылает событие о найденном нарушении.
void AnalyzerEngine::notifyIssue(const Issue& issue) const
{
    IssueEvent event;
    event.filePath = issue.file.generic_string();
    event.issue = issue;
    for (auto* obs : observers_) {
        if (obs) {
            obs->onIssue(event);
        }
    }
}

// Рассылает событие завершения анализа файла.
void AnalyzerEngine::notifyFileEnd(const std::string& path) const
{
    for (auto* obs : observers_) {
        if (obs) {
            obs->onFileEnd(path);
        }
    }
}

// Рассылает событие завершения анализа проекта.
void AnalyzerEngine::notifyAnalysisFinished(const AnalysisResult& result) const
{
    for (auto* obs : observers_) {
        if (obs) {
            obs->onAnalysisFinished(result);
        }
    }
}

// Выполняет полный цикл анализа: файлы, токены, правила и итог.
AnalysisResult AnalyzerEngine::analyzeProject(const std::filesystem::path& root, const Config& config) const
{
    AnalysisResult result;
    result.setIssueCallback([this](const Issue& issue) { notifyIssue(issue); });

    RuleFactory factory;
    const auto rules = factory.createFromConfig(config);

    FileScanner::ScanOptions scanOptions;
    scanOptions.extensions = config.extensions;
    scanOptions.excludedDirs = config.excludedDirs;
    scanOptions.excludedFiles = config.excludedFiles;
    std::vector<std::string> scanWarnings;
    scanOptions.warnings = &scanWarnings;

    const auto files = FileScanner::scan(root, scanOptions);
    result.filesChecked = files.size();

    for (const auto& warning : scanWarnings) {
        ++result.skippedFiles;
        result.addIssue(Issue{Severity::Warning, root, 0, 0, "CORE-FILE-SCANNER", warning,
            "Проверьте путь проекта, права доступа или правила исключения."});
    }

    Tokenizer tokenizer;
    for (const auto& filePath : files) {
        const auto filePathStr = filePath.lexically_normal().generic_string();
        notifyFileStart(filePathStr);

        std::ifstream in(filePath, std::ios::binary);
        if (!in) {
            ++result.failedFiles;
            result.addIssue(Issue{Severity::Warning, filePath, 0, 0, "CORE-IO",
                "Не удалось прочитать файл.",
                "Проверьте права доступа к файлу и кодировку."});
            notifyFileEnd(filePathStr);
            continue;
        }

        std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        auto tokenRes = tokenizer.tokenize(content);
        for (const auto& w : tokenRes.warnings) {
            result.addIssue(Issue{Severity::Warning, filePath, w.line, w.column, "CORE-TOKENIZER", w.message,
                "Упростите конструкцию или проверьте корректность литералов и комментариев."});
        }

        FileContext ctx;
        ctx.path = filePath.lexically_normal();
        ctx.lines = splitLines(content);
        ctx.tokens = std::move(tokenRes.tokens);

        for (const auto& rule : rules) {
            rule->apply(ctx, config, result);
        }

        notifyFileEnd(filePathStr);
    }

    notifyAnalysisFinished(result);
    result.setIssueCallback({});
    return result;
}
