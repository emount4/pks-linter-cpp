#pragma once

#include "AnalysisResult.h"
#include "Config.h"

#include <filesystem>
#include <vector>

class IObserver;

class AnalyzerEngine {
public:
    // Добавляет наблюдателя, который будет получать события анализа.
    void addObserver(IObserver* obs);

    // Запускает анализ проекта и возвращает накопленный результат.
    AnalysisResult analyzeProject(const std::filesystem::path& root, const Config& config) const;

private:
    // Уведомляет наблюдателей о начале анализа файла.
    void notifyFileStart(const std::string& path) const;

    // Уведомляет наблюдателей о найденном нарушении.
    void notifyIssue(const Issue& issue) const;

    // Уведомляет наблюдателей о завершении анализа файла.
    void notifyFileEnd(const std::string& path) const;

    // Уведомляет наблюдателей о завершении всего анализа.
    void notifyAnalysisFinished(const AnalysisResult& result) const;

    std::vector<IObserver*> observers_;
};
