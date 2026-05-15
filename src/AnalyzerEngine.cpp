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
    // If file ends with trailing newline, std::getline will not add empty last line.
    // That's fine for our checks.
    return lines;
}

} // namespace

void AnalyzerEngine::addObserver(IObserver* obs)
{
    observers_.push_back(obs);
}

AnalysisResult AnalyzerEngine::analyzeProject(const std::filesystem::path& root, const Config& config) const
{
    AnalysisResult result;
    result.setIssueCallback([this](const Issue& issue) {
        IssueEvent event;
        event.filePath = issue.file.generic_string();
        event.issue = issue;

        for (auto* obs : observers_) {
            if (obs) {
                obs->onIssue(event);
            }
        }
    });

    RuleFactory factory;
    const auto rules = factory.createFromConfig(config);

    FileScanner::ScanOptions scanOptions;
    scanOptions.extensions = config.extensions;
    scanOptions.excludedDirs = config.excludedDirs;

    const auto files = FileScanner::scan(root, scanOptions);
    result.filesChecked = files.size();

    Tokenizer tokenizer;

    for (const auto& filePath : files) {
        const auto filePathStr = filePath.generic_string();
        for (auto* obs : observers_) {
            if (obs) {
                obs->onFileStart(filePathStr);
            }
        }

        std::ifstream in(filePath, std::ios::binary);
        if (!in) {
            result.addIssue(Issue{Severity::Warning,
                filePath,
                0,
                0,
                "CORE-IO",
                "Не удалось прочитать файл.",
                "Проверьте права доступа и кодировку файла (ожидается UTF-8)."});

            for (auto* obs : observers_) {
                if (obs) {
                    obs->onFileEnd(filePathStr);
                }
            }
            continue;
        }

        std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        auto lines = splitLines(content);

        auto tokenRes = tokenizer.tokenize(content);
        for (const auto& w : tokenRes.warnings) {
            result.addIssue(Issue{Severity::Warning,
                filePath,
                w.line,
                w.column,
                "CORE-TOKENIZER",
                w.message,
                "Упростите конструкцию или проверьте корректность литералов/комментариев."});
        }

        FileContext ctx;
        ctx.path = filePath;
        ctx.lines = std::move(lines);
        ctx.tokens = std::move(tokenRes.tokens);

        for (const auto& rule : rules) {
            rule->apply(ctx, config, result);
        }

        for (auto* obs : observers_) {
            if (obs) {
                obs->onFileEnd(filePathStr);
            }
        }
    }

    for (auto* obs : observers_) {
        if (obs) {
            obs->onAnalysisFinished(result);
        }
    }

    result.setIssueCallback({});

    return result;
}
