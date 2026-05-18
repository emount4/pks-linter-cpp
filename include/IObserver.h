#pragma once

#include "AnalysisResult.h"
#include "IssueEvent.h"

#include <string>

class IObserver {
public:
    // Обеспечивает корректное удаление наблюдателей через базовый интерфейс.
    virtual ~IObserver() = default;

    // Обрабатывает событие о найденном нарушении.
    virtual void onIssue(const IssueEvent& event) = 0;

    // Обрабатывает событие начала анализа файла.
    virtual void onFileStart(const std::string& path) = 0;

    // Обрабатывает событие завершения анализа файла.
    virtual void onFileEnd(const std::string& path) = 0;

    // Обрабатывает событие завершения анализа всего проекта.
    virtual void onAnalysisFinished(const AnalysisResult&) {}
};
