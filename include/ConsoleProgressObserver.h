#pragma once

#include "IObserver.h"

#include <cstddef>
#include <ostream>

class ConsoleProgressObserver final : public IObserver {
public:
    // Создает наблюдателя прогресса с заданным потоком вывода.
    explicit ConsoleProgressObserver(std::ostream& os)
        : os_(os)
    {
    }

    // Игнорирует отдельные нарушения: прогресс показывает только ход анализа.
    void onIssue(const IssueEvent&) override {}

    // Печатает имя файла, который начал анализироваться.
    void onFileStart(const std::string& path) override
    {
        os_ << "[прогресс] анализ файла: " << path << '\n';
    }

    // Увеличивает счетчик обработанных файлов и печатает прогресс.
    void onFileEnd(const std::string&) override
    {
        ++processed_;
        os_ << "[прогресс] обработано файлов: " << processed_ << '\n';
    }

    // Печатает итоговую краткую сводку после завершения анализа.
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
