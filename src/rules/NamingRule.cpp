#include "rules/NamingRule.h"

#include <cctype>
#include <sstream>
#include <string_view>
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

bool isLowerCamel(const std::string& s)
{
    if (s.empty()) {
        return false;
    }
    if (std::islower(static_cast<unsigned char>(s[0])) == 0) {
        return false;
    }
    for (char c : s) {
        if (c == '_') {
            return false;
        }
        if (std::isalnum(static_cast<unsigned char>(c)) == 0) {
            return false;
        }
    }
    return true;
}

bool isUpperCamel(const std::string& s)
{
    if (s.empty()) {
        return false;
    }
    if (std::isupper(static_cast<unsigned char>(s[0])) == 0) {
        return false;
    }
    for (char c : s) {
        if (c == '_') {
            return false;
        }
        if (std::isalnum(static_cast<unsigned char>(c)) == 0) {
            return false;
        }
    }
    return true;
}

bool isLowerSnake(const std::string& s)
{
    if (s.empty()) {
        return false;
    }
    if (std::islower(static_cast<unsigned char>(s[0])) == 0) {
        return false;
    }
    for (char c : s) {
        if (c == '_') {
            continue;
        }
        if (std::isdigit(static_cast<unsigned char>(c)) != 0) {
            continue;
        }
        if (std::islower(static_cast<unsigned char>(c)) == 0) {
            return false;
        }
    }
    return true;
}

bool isUpperSnake(const std::string& s)
{
    if (s.empty()) {
        return false;
    }
    if (std::isupper(static_cast<unsigned char>(s[0])) == 0) {
        return false;
    }
    for (char c : s) {
        if (c == '_') {
            continue;
        }
        if (std::isdigit(static_cast<unsigned char>(c)) != 0) {
            continue;
        }
        if (std::isupper(static_cast<unsigned char>(c)) == 0) {
            return false;
        }
    }
    return true;
}

bool matchesStyle(const std::string& name, NamingStyle style)
{
    switch (style) {
    case NamingStyle::Any:
        return true;
    case NamingStyle::LowerCamel:
        return isLowerCamel(name);
    case NamingStyle::UpperCamel:
        return isUpperCamel(name);
    case NamingStyle::LowerSnake:
        return isLowerSnake(name);
    case NamingStyle::UpperSnake:
        return isUpperSnake(name);
    }
    return true;
}

std::string styleLabel(NamingStyle style)
{
    switch (style) {
    case NamingStyle::Any:
        return "any";
    case NamingStyle::LowerCamel:
        return "lowerCamelCase";
    case NamingStyle::UpperCamel:
        return "UpperCamelCase";
    case NamingStyle::LowerSnake:
        return "snake_case";
    case NamingStyle::UpperSnake:
        return "UPPER_SNAKE_CASE";
    }
    return "any";
}

bool isTypeKeyword(const Token& t)
{
    if (t.kind != TokenKind::Keyword) {
        return false;
    }
    static const std::unordered_set<std::string> types{
        "void", "bool", "char", "short", "int", "long", "float", "double", "signed", "unsigned", "auto", "wchar_t",
        "char16_t", "char32_t"};
    return types.find(t.lexeme) != types.end();
}

bool isQualifier(const Token& t)
{
    if (t.kind != TokenKind::Keyword) {
        return false;
    }
    static const std::unordered_set<std::string> qs{"const", "constexpr", "static", "volatile", "mutable", "inline"};
    return qs.find(t.lexeme) != qs.end();
}

// Parse simple variable declaration starting at idx.
// Returns true if consumed a declaration ending with ';' and outputs found variable names.
bool tryParseVarDecl(const std::vector<Token>& toks, std::size_t idx, std::vector<std::pair<Token, bool>>& outVars)
{
    std::size_t i = idx;

    bool isConst = false;
    while (i < toks.size() && isQualifier(toks[i])) {
        if (toks[i].lexeme == "const" || toks[i].lexeme == "constexpr") {
            isConst = true;
        }
        ++i;
    }

    // type
    if (i >= toks.size()) {
        return false;
    }

    // support qualified types: Ident (:: Ident)*
    if (!(isTypeKeyword(toks[i]) || toks[i].kind == TokenKind::Identifier)) {
        return false;
    }

    ++i;
    while (i + 1 < toks.size() && toks[i].kind == TokenKind::Operator && toks[i].lexeme == "::" &&
        toks[i + 1].kind == TokenKind::Identifier) {
        i += 2;
    }

    // declarators
    bool foundAny = false;
    for (;;) {
        while (i < toks.size() && toks[i].kind == TokenKind::Operator && (toks[i].lexeme == "*" || toks[i].lexeme == "&")) {
            ++i;
        }

        if (i >= toks.size() || toks[i].kind != TokenKind::Identifier) {
            return false;
        }

        Token nameTok = toks[i];
        ++i;

        // arrays: name[...]
        if (i < toks.size() && toks[i].lexeme == "[") {
            int depth = 0;
            while (i < toks.size()) {
                if (toks[i].lexeme == "[") {
                    ++depth;
                } else if (toks[i].lexeme == "]") {
                    --depth;
                    if (depth == 0) {
                        ++i;
                        break;
                    }
                }
                ++i;
            }
        }

        outVars.push_back({nameTok, isConst});
        foundAny = true;

        // Skip initializer (best-effort) until ',' or ';' at depth 0.
        if (i < toks.size() && (toks[i].lexeme == "=" || toks[i].lexeme == "{" || toks[i].lexeme == "(")) {
            int paren = 0;
            int brace = 0;
            int bracket = 0;
            while (i < toks.size()) {
                if (toks[i].lexeme == "(") {
                    ++paren;
                } else if (toks[i].lexeme == ")") {
                    paren = std::max(0, paren - 1);
                } else if (toks[i].lexeme == "{") {
                    ++brace;
                } else if (toks[i].lexeme == "}") {
                    brace = std::max(0, brace - 1);
                } else if (toks[i].lexeme == "[") {
                    ++bracket;
                } else if (toks[i].lexeme == "]") {
                    bracket = std::max(0, bracket - 1);
                }

                if (paren == 0 && brace == 0 && bracket == 0 && (toks[i].lexeme == "," || toks[i].lexeme == ";")) {
                    break;
                }
                ++i;
            }
        }

        if (i >= toks.size()) {
            return false;
        }

        if (toks[i].lexeme == ",") {
            ++i;
            continue;
        }
        if (toks[i].lexeme == ";") {
            return foundAny;
        }
        return false;
    }
}

} // namespace

void NamingRule::apply(const FileContext& file, const Config& config, AnalysisResult& result) const
{
    // #define constants
    for (std::size_t i = 0; i < file.lines.size(); ++i) {
        auto t = ltrimCopy(file.lines[i]);
        if (t.rfind("#define", 0) != 0) {
            continue;
        }
        std::stringstream ss(t);
        std::string kw;
        std::string name;
        ss >> kw >> name;
        if (name.empty()) {
            continue;
        }
        if (!matchesStyle(name, config.constantNaming)) {
            result.addIssue(Issue{Severity::Warning,
                file.path,
                static_cast<int>(i + 1),
                1,
                id(),
                "Имя константы/макроса '" + name + "' не соответствует стилю " + styleLabel(config.constantNaming) + ".",
                "Переименуйте константу/макрос в соответствии с выбранным стилем."});
        }
    }

    // Functions (global scope) and variable declarations (best-effort)
    int braceDepth = 0;

    for (std::size_t i = 0; i < file.tokens.size(); ++i) {
        const auto& tok = file.tokens[i];
        if (tok.lexeme == "{") {
            ++braceDepth;
        } else if (tok.lexeme == "}") {
            braceDepth = std::max(0, braceDepth - 1);
        }

        // Function name: identifier before '(' whose next non-trivia after matching ')' is '{'
        if (braceDepth == 0 && tok.lexeme == "(" && i > 0) {
            // find name token before '('
            std::size_t nameIdx = i;
            while (nameIdx > 0) {
                --nameIdx;
                if (file.tokens[nameIdx].kind == TokenKind::Identifier) {
                    break;
                }
                if (file.tokens[nameIdx].lexeme == "::") {
                    continue;
                }
                if (file.tokens[nameIdx].kind == TokenKind::Keyword) {
                    nameIdx = i; // invalid
                    break;
                }
            }
            if (nameIdx >= i || file.tokens[nameIdx].kind != TokenKind::Identifier) {
                continue;
            }

            // Skip control keywords: if/for/while/switch/catch
            if (nameIdx > 0 && file.tokens[nameIdx - 1].kind == TokenKind::Keyword) {
                auto prev = file.tokens[nameIdx - 1].lexeme;
                if (prev == "if" || prev == "for" || prev == "while" || prev == "switch" || prev == "catch") {
                    continue;
                }
            }

            // Find matching ')'
            int paren = 1;
            std::size_t j = i + 1;
            for (; j < file.tokens.size(); ++j) {
                if (file.tokens[j].lexeme == "(") {
                    ++paren;
                } else if (file.tokens[j].lexeme == ")") {
                    --paren;
                    if (paren == 0) {
                        break;
                    }
                }
            }
            if (j >= file.tokens.size()) {
                continue;
            }

            // Next token after ')' skipping qualifiers like const/noexcept
            std::size_t k = j + 1;
            while (k < file.tokens.size()) {
                auto lx = file.tokens[k].lexeme;
                if (lx == "const" || lx == "noexcept") {
                    ++k;
                    continue;
                }
                break;
            }
            if (k < file.tokens.size() && file.tokens[k].lexeme == "{") {
                const auto& fn = file.tokens[nameIdx];
                if (!matchesStyle(fn.lexeme, config.functionNaming)) {
                    result.addIssue(Issue{Severity::Warning,
                        file.path,
                        fn.line,
                        fn.column,
                        id(),
                        "Имя функции '" + fn.lexeme + "' не соответствует стилю " + styleLabel(config.functionNaming) + ".",
                        "Переименуйте функцию в соответствии с выбранным стилем."});
                }
            }
        }

        // Variable declarations: scan at statement starts (rough)
        if (tok.lexeme == ";" || tok.lexeme == "{" || tok.lexeme == "}") {
            std::size_t start = i + 1;
            if (start >= file.tokens.size()) {
                continue;
            }
            std::vector<std::pair<Token, bool>> vars;
            if (tryParseVarDecl(file.tokens, start, vars)) {
                for (const auto& [nameTok, isConst] : vars) {
                    NamingStyle style = isConst ? config.constantNaming : config.variableNaming;
                    if (!matchesStyle(nameTok.lexeme, style)) {
                        result.addIssue(Issue{Severity::Warning,
                            file.path,
                            nameTok.line,
                            nameTok.column,
                            id(),
                            "Имя переменной '" + nameTok.lexeme + "' не соответствует стилю " + styleLabel(style) + ".",
                            "Переименуйте идентификатор в соответствии с выбранным стилем."});
                    }
                }
            }
        }
    }
}
