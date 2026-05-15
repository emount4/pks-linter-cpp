#pragma once

#include "Issue.h"

#include <string>

struct IssueEvent {
    std::string filePath;
    Issue issue;
};
