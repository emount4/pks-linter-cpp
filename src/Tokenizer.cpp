#include "Tokenizer.h"

#include <cctype>
#include <unordered_set>

namespace {

bool isIdentStart(char c)
{
    return std::isalpha(static_cast<unsigned char>(c)) != 0 || c == '_';
}

bool isIdentChar(char c)
{
    return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

const std::unordered_set<std::string>& keywords()
{
    static const std::unordered_set<std::string> k{
        "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool", "break",
        "case", "catch", "char", "char8_t", "char16_t", "char32_t", "class", "compl", "concept",
        "const", "consteval", "constexpr", "constinit", "const_cast", "continue", "co_await", "co_return",
        "co_yield", "decltype", "default", "delete", "do", "double", "dynamic_cast", "else", "enum",
        "explicit", "export", "extern", "false", "float", "for", "friend", "goto", "if", "inline",
        "int", "long", "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator",
        "or", "or_eq", "private", "protected", "public", "register", "reinterpret_cast", "requires",
        "return", "short", "signed", "sizeof", "static", "static_assert", "static_cast", "string", "struct",
        "switch", "template", "this", "thread_local", "throw", "true", "try", "typedef", "typeid",
        "typename", "union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t", "while",
        "xor", "xor_eq"};
    return k;
}

bool matchAt(const std::string& input, std::size_t pos, const std::string& s)
{
    if (pos + s.size() > input.size()) {
        return false;
    }
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (input[pos + i] != s[i]) {
            return false;
        }
    }
    return true;
}

}

TokenizationResult Tokenizer::tokenize(const std::string& input) const
{
    TokenizationResult res;

    int line = 1;
    int col = 1;

    auto push = [&](TokenKind kind, std::string lex, int l, int c) {
        Token t;
        t.kind = kind;
        t.lexeme = std::move(lex);
        t.line = l;
        t.column = c;
        res.tokens.push_back(std::move(t));
    };

    const std::string ops[] = {
        "<<=", ">>=", "::", "->", "==", "!=", "<=", ">=", "&&", "||", "++", "--", "+=", "-=", "*=",
        "/=", "%=", "<<", ">>", "=", "+", "-", "*", "/", "%", "<", ">", "!", "&", "|", "^", "~"};

    for (std::size_t i = 0; i < input.size();) {
        char c = input[i];

        if (c == '\n') {
            ++line;
            col = 1;
            ++i;
            continue;
        }

        if (c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\v') {
            ++col;
            ++i;
            continue;
        }

        // Комментарии
        if (c == '/' && i + 1 < input.size() && input[i + 1] == '/') {
            int l = line;
            int c0 = col;
            std::string lex = "//";
            i += 2;
            col += 2;
            while (i < input.size() && input[i] != '\n') {
                lex.push_back(input[i]);
                ++i;
                ++col;
            }
            push(TokenKind::Comment, std::move(lex), l, c0);
            continue;
        }
        if (c == '/' && i + 1 < input.size() && input[i + 1] == '*') {
            int startLine = line;
            int startCol = col;
            std::string lex = "/*";
            i += 2;
            col += 2;
            bool closed = false;
            while (i < input.size()) {
                if (input[i] == '\n') {
                    lex.push_back(input[i]);
                    ++line;
                    col = 1;
                    ++i;
                    continue;
                }
                if (input[i] == '*' && i + 1 < input.size() && input[i + 1] == '/') {
                    lex.push_back('*');
                    lex.push_back('/');
                    i += 2;
                    col += 2;
                    closed = true;
                    break;
                }
                lex.push_back(input[i]);
                ++i;
                ++col;
            }
            if (!closed) {
                res.warnings.push_back({startLine, startCol, "Незакрытый блочный комментарий"});
            }
            push(TokenKind::Comment, std::move(lex), startLine, startCol);
            continue;
        }

        // Строковый литерал
        if (c == '"') {
            int l = line;
            int c0 = col;
            std::string lex;
            lex.push_back(c);
            ++i;
            ++col;
            bool closed = false;
            while (i < input.size()) {
                char ch = input[i];
                if (ch == '\n') {
                    break; // считаем литерал незакрытым
                }
                lex.push_back(ch);
                ++i;
                ++col;
                if (ch == '\\' && i < input.size()) {
                    // экранируем следующий символ
                    lex.push_back(input[i]);
                    ++i;
                    ++col;
                    continue;
                }
                if (ch == '"') {
                    closed = true;
                    break;
                }
            }
            if (!closed) {
                res.warnings.push_back({l, c0, "Незакрытый строковый литерал"});
            }
            push(TokenKind::StringLiteral, std::move(lex), l, c0);
            continue;
        }

        // Символьный литерал
        if (c == '\'') {
            int l = line;
            int c0 = col;
            std::string lex;
            lex.push_back(c);
            ++i;
            ++col;
            bool closed = false;
            while (i < input.size()) {
                char ch = input[i];
                if (ch == '\n') {
                    break;
                }
                lex.push_back(ch);
                ++i;
                ++col;
                if (ch == '\\' && i < input.size()) {
                    lex.push_back(input[i]);
                    ++i;
                    ++col;
                    continue;
                }
                if (ch == '\'') {
                    closed = true;
                    break;
                }
            }
            if (!closed) {
                res.warnings.push_back({l, c0, "Незакрытый символьный литерал"});
            }
            push(TokenKind::CharLiteral, std::move(lex), l, c0);
            continue;
        }

        // Идентификатор или ключевое слово
        if (isIdentStart(c)) {
            int l = line;
            int c0 = col;
            std::string lex;
            lex.push_back(c);
            ++i;
            ++col;
            while (i < input.size() && isIdentChar(input[i])) {
                lex.push_back(input[i]);
                ++i;
                ++col;
            }
            if (keywords().find(lex) != keywords().end()) {
                push(TokenKind::Keyword, std::move(lex), l, c0);
            } else {
                push(TokenKind::Identifier, std::move(lex), l, c0);
            }
            continue;
        }

        // Число
        if (std::isdigit(static_cast<unsigned char>(c)) != 0) {
            int l = line;
            int c0 = col;
            std::string lex;
            lex.push_back(c);
            ++i;
            ++col;
            while (i < input.size()) {
                char ch = input[i];
                if (std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '.' || ch == '_') {
                    lex.push_back(ch);
                    ++i;
                    ++col;
                } else {
                    break;
                }
            }
            push(TokenKind::Number, std::move(lex), l, c0);
            continue;
        }

        // Операторы
        bool matched = false;
        for (const auto& op : ops) {
            if (matchAt(input, i, op)) {
                push(TokenKind::Operator, op, line, col);
                i += op.size();
                col += static_cast<int>(op.size());
                matched = true;
                break;
            }
        }
        if (matched) {
            continue;
        }

        // Пунктуация
        {
            int l = line;
            int c0 = col;
            std::string lex(1, c);
            push(TokenKind::Punctuation, std::move(lex), l, c0);
            ++i;
            ++col;
        }
    }

    return res;
}
