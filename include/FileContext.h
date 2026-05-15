#pragma once

#include "Tokenizer.h"

#include <filesystem>
#include <string>
#include <vector>

struct FileContext {
    std::filesystem::path path;
    std::vector<std::string> lines;
    std::vector<Token> tokens;
};
