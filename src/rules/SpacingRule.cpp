#include "rules/SpacingRule.h"

#include <cctype>
#include <unordered_set>

namespace {

std::string ltrimCopy(const std::string& s)
{
    std::size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i])) != 0) {
        ++i;
    }
    return s.substr(i);
}

bool isPreprocessorLine(const FileContext& file, int line)
{
    if (line <= 0 || static_cast<std::size_t>(line) > file.lines.size()) {
        return false;
    }
    auto t = ltrimCopy(file.lines[static_cast<std::size_t>(line - 1)]);
    return !t.empty() && t[0] == '#';
}

bool isWhitespace(char c)
{
    return c == ' ' || c == '\t';
}

} // namespace

void SpacingRule::apply(const FileContext& file, const Config&, AnalysisResult& result) const
{
    const std::unordered_set<std::string> ops{
        "=", "+", "-", "*", "/", "%", "==", "!=", "<=", ">=", "<", ">", "&&", "||", "<<", ">>"};

    for (const auto& tok : file.tokens) {
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

        // Comma: require space after
        if (tok.kind == TokenKind::Punctuation && tok.lexeme == ",") {
            const std::size_t idx = static_cast<std::size_t>(start);
            if (idx + 1 < line.size() && !isWhitespace(line[idx + 1])) {
                result.addIssue(Issue{Severity::Warning,
                    file.path,
                    tok.line,
                    tok.column,
                    id(),
                    "После запятой отсутствует пробел.",
                    "Добавьте пробел после запятой (например: a, b)."});
            }
            continue;
        }

        // Operators: require whitespace on both sides (simple heuristic)
        if (tok.kind == TokenKind::Operator && ops.find(tok.lexeme) != ops.end()) {
            const std::size_t idx0 = static_cast<std::size_t>(start);
            const std::size_t idx1 = idx0 + tok.lexeme.size();

            if (idx0 == 0 || idx1 >= line.size()) {
                continue;
            }

            // Heuristic: skip likely unary +/-, * deref, & address-of
            auto prevNonSpace = [&](std::size_t from) -> char {
                while (from > 0) {
                    --from;
                    if (!isWhitespace(line[from])) {
                        return line[from];
                    }
                }
                return '\0';
            };

            char prev = prevNonSpace(idx0);
            if ((tok.lexeme == "+" || tok.lexeme == "-") && (prev == '(' || prev == '[' || prev == '{' || prev == ',' || prev == '=')) {
                continue;
            }
            if ((tok.lexeme == "*" || tok.lexeme == "&") && (prev == '(' || prev == '[' || prev == '{' || prev == ',' || prev == '=')) {
                continue;
            }

            const char left = line[idx0 - 1];
            const char right = line[idx1];
            if (!isWhitespace(left) || !isWhitespace(right)) {
                result.addIssue(Issue{Severity::Warning,
                    file.path,
                    tok.line,
                    tok.column,
                    id(),
                    "Отсутствуют пробелы вокруг оператора '" + tok.lexeme + "'.",
                    "Добавьте пробелы вокруг оператора (например: a " + tok.lexeme + " b)."});
            }
        }
    }
}
