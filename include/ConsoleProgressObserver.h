#pragma once

#include "IObserver.h"

#include <cstddef>
#include <ostream>

class ConsoleProgressObserver final : public IObserver {
public:
    explicit ConsoleProgressObserver(std::ostream& os)
        : os_(os)
    {
    }

    void onIssue(const IssueEvent&) override {}

    void onFileStart(const std::string& path) override
    {
        os_ << "[прогресс] анализ файла: " << path << '\n';
    }

    void onFileEnd(const std::string&) override
    {
        ++processed_;
        os_ << "[прогресс] обработано файлов: " << processed_ << '\n';
    }

    void onAnalysisFinished(const AnalysisResult& result) override
    {
        os_ << "[прогресс] завершено: проверено файлов: " << result.filesChecked
            << ", предупреждений: " << result.warningCount()
            << ", ошибок: " << result.errorCount() << '\n';
    }

private:
    std::ostream& os_;
    std::size_t processed_{0};
};
