#pragma once

#include "AnalysisResult.h"

#include <ostream>

class Reporter {
public:
    static void print(const AnalysisResult& result, std::ostream& os);
};
