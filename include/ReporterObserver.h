#pragma once

#include "IObserver.h"
#include "Reporter.h"

#include <ostream>

class ReporterObserver final : public IObserver {
public:
    // Создает наблюдателя, который печатает итоговый отчет в заданный поток.
    explicit ReporterObserver(std::ostream& os)
        : os_(os)
    {
    }

    // Игнорирует отдельные нарушения: отчет печатается только в конце.
    void onIssue(const IssueEvent&) override {}

    // Игнорирует начало файла: это не нужно итоговому отчету.
    void onFileStart(const std::string&) override {}

    // Игнорирует завершение файла: это не нужно итоговому отчету.
    void onFileEnd(const std::string&) override {}

    // Печатает итоговый отчет после завершения анализа.
    void onAnalysisFinished(const AnalysisResult& result) override
    {
        Reporter::print(result, os_);
    }

private:
    std::ostream& os_;
};
