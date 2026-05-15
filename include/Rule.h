#pragma once

#include "AnalysisResult.h"
#include "Config.h"
#include "FileContext.h"

#include <string>

class Rule {
public:
    virtual ~Rule() = default;

    virtual std::string id() const = 0;
    virtual std::string name() const = 0;

    virtual void apply(const FileContext& file, const Config& config, AnalysisResult& result) const = 0;
};
