#include <catch2/catch_test_macros.hpp>

#include "Tokenizer.h"

TEST_CASE("Tokenizer recognizes identifiers, keywords and numbers")
{
    Tokenizer tokenizer;
    auto result = tokenizer.tokenize("int value = 42;\nreturn value;");

    REQUIRE(result.warnings.empty());
    REQUIRE(result.tokens.size() >= 6);
    REQUIRE(result.tokens[0].kind == TokenKind::Keyword);
    REQUIRE(result.tokens[0].lexeme == "int");
    REQUIRE(result.tokens[1].kind == TokenKind::Identifier);
    REQUIRE(result.tokens[1].lexeme == "value");
    REQUIRE(result.tokens[3].kind == TokenKind::Number);
    REQUIRE(result.tokens[3].lexeme == "42");
}

TEST_CASE("Tokenizer recognizes string and char literals")
{
    Tokenizer tokenizer;
    auto result = tokenizer.tokenize("const char* s = \"hi\"; char c = 'x';");

    REQUIRE(result.warnings.empty());
    bool foundString = false;
    bool foundChar = false;
    for (const auto& token : result.tokens) {
        if (token.kind == TokenKind::StringLiteral) {
            foundString = true;
            REQUIRE(token.lexeme == "\"hi\"");
        }
        if (token.kind == TokenKind::CharLiteral) {
            foundChar = true;
            REQUIRE(token.lexeme == "'x'");
        }
    }
    REQUIRE(foundString);
    REQUIRE(foundChar);
}

TEST_CASE("Tokenizer skips line and block comments")
{
    Tokenizer tokenizer;
    auto result = tokenizer.tokenize("int a = 1; // comment\n/* block */ int b = 2;");

    REQUIRE(result.warnings.empty());
    REQUIRE(result.tokens.size() > 0);
    for (const auto& token : result.tokens) {
        REQUIRE(token.lexeme != "comment");
        REQUIRE(token.lexeme != "block");
    }
}

TEST_CASE("Tokenizer warns on unclosed string literal")
{
    Tokenizer tokenizer;
    auto result = tokenizer.tokenize("const char* s = \"broken;");

    REQUIRE(result.tokens.size() > 0);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].message == "Unclosed string literal");
}

TEST_CASE("Tokenizer warns on unclosed block comment")
{
    Tokenizer tokenizer;
    auto result = tokenizer.tokenize("/* broken comment");

    REQUIRE(result.tokens.empty());
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].message == "Unclosed block comment");
}

TEST_CASE("Tokenizer records operator and punctuation tokens with positions")
{
    Tokenizer tokenizer;
    auto result = tokenizer.tokenize("a+b, c->d");

    REQUIRE(result.warnings.empty());
    bool foundPlus = false;
    bool foundComma = false;
    bool foundArrow = false;
    for (const auto& token : result.tokens) {
        if (token.lexeme == "+") {
            foundPlus = true;
            REQUIRE(token.kind == TokenKind::Operator);
            REQUIRE(token.column == 2);
        }
        if (token.lexeme == ",") {
            foundComma = true;
            REQUIRE(token.kind == TokenKind::Punctuation);
        }
        if (token.lexeme == "->") {
            foundArrow = true;
            REQUIRE(token.kind == TokenKind::Operator);
        }
    }
    REQUIRE(foundPlus);
    REQUIRE(foundComma);
    REQUIRE(foundArrow);
}
