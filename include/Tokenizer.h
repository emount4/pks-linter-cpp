#pragma once

#include <string>
#include <vector>

enum class TokenKind {
    Identifier,
    Keyword,
    Number,
    StringLiteral,
    CharLiteral,
    Operator,
    Punctuation,
};

struct Token {
    TokenKind kind{TokenKind::Identifier};
    std::string lexeme;
    int line{1};
    int column{1};
};

struct TokenizationWarning {
    int line{1};
    int column{1};
    std::string message;
};

struct TokenizationResult {
    std::vector<Token> tokens;
    std::vector<TokenizationWarning> warnings;
};

class Tokenizer {
public:
    TokenizationResult tokenize(const std::string& input) const;
};
