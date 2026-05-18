#include "rules/SpacingRule.h"

#include "SeverityUtil.h"

#include <cctype>
#include <unordered_set>

namespace {

// Возвращает копию строки без начальных пробелов.
std::string ltrimCopy(const std::string& s)
{
    std::size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i])) != 0) {
        ++i;
    }
    return s.substr(i);
}

// Проверяет, является ли строка препроцессорной директивой.
bool isPreprocessorLine(const FileContext& file, int line)
{
    if (line <= 0 || static_cast<std::size_t>(line) > file.lines.size()) {
        return false;
    }
    auto t = ltrimCopy(file.lines[static_cast<std::size_t>(line - 1)]);
    return !t.empty() && t[0] == '#';
}

// Проверяет обычный пробел или табуляцию.
bool isWhitespace(char c)
{
    return c == ' ' || c == '\t';
}

// Эвристически определяет, похож ли токен на имя типа.
bool isTypeLike(const Token& t)
{
    if (t.kind == TokenKind::Keyword) {
        static const std::unordered_set<std::string> types{
            "int", "double", "float", "char", "bool", "string", "void", "auto", "long", "short", "unsigned", "signed"};
        return types.find(t.lexeme) != types.end();
    }
    return t.kind == TokenKind::Identifier && !t.lexeme.empty() &&
        std::isupper(static_cast<unsigned char>(t.lexeme.front())) != 0;
}

// Проверяет, стоит ли оператор в унарном контексте.
bool isUnaryContext(const std::vector<Token>& tokens, std::size_t index)
{
    if (index == 0) {
        return true;
    }
    const auto& prev = tokens[index - 1].lexeme;
    return prev == "(" || prev == "[" || prev == "{" || prev == "," || prev == "=" || prev == "return" ||
        prev == "+" || prev == "-" || prev == "*" || prev == "/" || prev == "%";
}

// Проверяет, является ли '*' или '&' частью объявления указателя/ссылки.
bool isPointerOrReferenceDeclarator(const std::vector<Token>& tokens, std::size_t index)
{
    if (index == 0 || index + 1 >= tokens.size()) {
        return false;
    }
    const auto& tok = tokens[index];
    if (!(tok.lexeme == "*" || tok.lexeme == "&")) {
        return false;
    }
    return isTypeLike(tokens[index - 1]) && tokens[index + 1].kind == TokenKind::Identifier;
}

} // пространство имен

// Проверяет пробелы вокруг операторов и после запятых.
void SpacingRule::apply(const FileContext& file, const Config& config, AnalysisResult& result) const
{
    const std::unordered_set<std::string> ops{
        "=", "+", "-", "*", "/", "%", "==", "!=", "<=", ">=", "<", ">", "&&", "||"};
    const auto severity = configuredSeverity(config, id());

    for (std::size_t tokenIndex = 0; tokenIndex < file.tokens.size(); ++tokenIndex) {
        const auto& tok = file.tokens[tokenIndex];
        if (tok.line <= 0 || static_cast<std::size_t>(tok.line) > file.lines.size()) {
            continue;
        }
        if (isPreprocessorLine(file, tok.line)) {
            continue;
        }

        const auto& line = file.lines[static_cast<std::size_t>(tok.line - 1)];
        const int start = tok.column - 1;
        if (start < 0 || static_cast<std::size_t>(start) >= line.size()) {
            continue;
        }

        if (tok.kind == TokenKind::Punctuation && tok.lexeme == ",") {
            const std::size_t idx = static_cast<std::size_t>(start);
            if (idx + 1 < line.size() && !isWhitespace(line[idx + 1])) {
                result.addIssue(Issue{severity, file.path, tok.line, tok.column, id(),
                    "После запятой отсутствует пробел.",
                    "Добавьте один пробел после запятой, например: a, b."});
            }
            continue;
        }

        if (tok.kind != TokenKind::Operator || ops.find(tok.lexeme) == ops.end()) {
            continue;
        }

        if ((tok.lexeme == "+" || tok.lexeme == "-") && isUnaryContext(file.tokens, tokenIndex)) {
            continue;
        }
        if ((tok.lexeme == "*" || tok.lexeme == "&") && isUnaryContext(file.tokens, tokenIndex)) {
            continue;
        }
        if (isPointerOrReferenceDeclarator(file.tokens, tokenIndex)) {
            continue;
        }

        const std::size_t idx0 = static_cast<std::size_t>(start);
        const std::size_t idx1 = idx0 + tok.lexeme.size();
        if (idx0 == 0 || idx1 >= line.size()) {
            continue;
        }

        const char left = line[idx0 - 1];
        const char right = line[idx1];
        if (!isWhitespace(left) || !isWhitespace(right)) {
            result.addIssue(Issue{severity, file.path, tok.line, tok.column, id(),
                "Отсутствуют пробелы вокруг бинарного оператора '" + tok.lexeme + "'.",
                "Добавьте пробелы вокруг оператора, например: a " + tok.lexeme + " b."});
        }
    }
}
