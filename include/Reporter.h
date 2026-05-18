#pragma once

#include "AnalysisResult.h"

#include <ostream>

class Reporter {
public:
    // Печатает отсортированный отчет анализа в указанный поток.
    static void print(const AnalysisResult& result, std::ostream& os);
};
