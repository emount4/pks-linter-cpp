#include "rules/UseBeforeInitRule.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

struct FunctionDef {
    std::string name;
    std::size_t bodyStart{0};
    std::size_t bodyEnd{0}; // inclusive index of matching '}'
};

bool isControlKeyword(const Token& t)
{
    if (t.kind != TokenKind::Keyword) {
        return false;
    }
    return t.lexeme == "if" || t.lexeme == "for" || t.lexeme == "while" || t.lexeme == "switch" || t.lexeme == "catch";
}

std::vector<FunctionDef> findFunctions(const std::vector<Token>& toks)
{
    std::vector<FunctionDef> out;

    int braceDepth = 0;
    for (std::size_t i = 0; i < toks.size(); ++i) {
        if (toks[i].lexeme == "{") {
            ++braceDepth;
            continue;
        }
        if (toks[i].lexeme == "}") {
            braceDepth = std::max(0, braceDepth - 1);
            continue;
        }

        if (braceDepth != 0) {
            continue;
        }

        if (toks[i].lexeme != "(" || i == 0) {
            continue;
        }

        // find function name as last identifier before '('
        std::size_t nameIdx = i;
        while (nameIdx > 0) {
            --nameIdx;
            if (toks[nameIdx].kind == TokenKind::Identifier) {
                break;
            }
            if (toks[nameIdx].lexeme == "::") {
                continue;
            }
            if (toks[nameIdx].kind == TokenKind::Keyword) {
                nameIdx = i;
                break;
            }
        }
        if (nameIdx >= i || toks[nameIdx].kind != TokenKind::Identifier) {
            continue;
        }
        if (nameIdx > 0 && isControlKeyword(toks[nameIdx - 1])) {
            continue;
        }

        // match ')'
        int paren = 1;
        std::size_t j = i + 1;
        for (; j < toks.size(); ++j) {
            if (toks[j].lexeme == "(") {
                ++paren;
            } else if (toks[j].lexeme == ")") {
                --paren;
                if (paren == 0) {
                    break;
                }
            }
        }
        if (j >= toks.size()) {
            continue;
        }

        // next '{'
        std::size_t k = j + 1;
        while (k < toks.size() && (toks[k].lexeme == "const" || toks[k].lexeme == "noexcept")) {
            ++k;
        }
        if (k >= toks.size() || toks[k].lexeme != "{") {
            continue;
        }

        // match closing '}'
        int depth = 1;
        std::size_t m = k + 1;
        for (; m < toks.size(); ++m) {
            if (toks[m].lexeme == "{") {
                ++depth;
            } else if (toks[m].lexeme == "}") {
                --depth;
                if (depth == 0) {
                    break;
                }
            }
        }
        if (m >= toks.size()) {
            continue;
        }

        FunctionDef fn;
        fn.name = toks[nameIdx].lexeme;
        fn.bodyStart = k + 1;
        fn.bodyEnd = m;
        out.push_back(std::move(fn));

        i = m; // skip body
    }

    return out;
}

bool isQualifier(const Token& t)
{
    return t.kind == TokenKind::Keyword && (t.lexeme == "const" || t.lexeme == "constexpr" || t.lexeme == "static" ||
                                              t.lexeme == "volatile" || t.lexeme == "mutable");
}

bool isTypeToken(const Token& t)
{
    if (t.kind == TokenKind::Keyword) {
        static const std::unordered_set<std::string> types{"void", "bool", "char", "short", "int", "long", "float",
            "double", "signed", "unsigned", "auto", "wchar_t", "char16_t", "char32_t"};
        return types.find(t.lexeme) != types.end();
    }
    // heuristic: user-defined type
    if (t.kind == TokenKind::Identifier) {
        return std::isupper(static_cast<unsigned char>(t.lexeme[0])) != 0;
    }
    return false;
}

struct DeclVar {
    Token nameTok;
    bool initialized{false};
};

bool tryParseDecl(const std::vector<Token>& toks, std::size_t idx, std::vector<DeclVar>& outVars, std::size_t* outNext)
{
    std::size_t i = idx;

    while (i < toks.size() && isQualifier(toks[i])) {
        ++i;
    }

    if (i >= toks.size() || !isTypeToken(toks[i])) {
        return false;
    }

    ++i;
    // qualified type: Ident :: Ident
    while (i + 1 < toks.size() && toks[i].kind == TokenKind::Operator && toks[i].lexeme == "::" &&
        toks[i + 1].kind == TokenKind::Identifier) {
        i += 2;
    }

    bool foundAny = false;

    for (;;) {
        while (i < toks.size() && toks[i].kind == TokenKind::Operator && (toks[i].lexeme == "*" || toks[i].lexeme == "&")) {
            ++i;
        }
        if (i >= toks.size() || toks[i].kind != TokenKind::Identifier) {
            return false;
        }

        DeclVar v;
        v.nameTok = toks[i];
        ++i;

        // array suffix
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

        if (i < toks.size() && (toks[i].lexeme == "=" || toks[i].lexeme == "{" || toks[i].lexeme == "(")) {
            v.initialized = true;
            // skip initializer tokens until ',' or ';' at depth 0
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

        outVars.push_back(v);
        foundAny = true;

        if (i >= toks.size()) {
            return false;
        }
        if (toks[i].lexeme == ",") {
            ++i;
            continue;
        }
        if (toks[i].lexeme == ";") {
            if (outNext) {
                *outNext = i + 1;
            }
            return foundAny;
        }
        return false;
    }
}

bool isAssignmentOp(const Token& t)
{
    if (t.kind != TokenKind::Operator) {
        return false;
    }
    return t.lexeme == "=" || t.lexeme == "+=" || t.lexeme == "-=" || t.lexeme == "*=" || t.lexeme == "/=" ||
        t.lexeme == "%=";
}

bool isMemberOrScopeAccess(const std::vector<Token>& toks, std::size_t idx)
{
    if (idx == 0) {
        return false;
    }
    auto prev = toks[idx - 1].lexeme;
    return prev == "." || prev == "->" || prev == "::";
}

struct VarInfo {
    bool initialized{false};
};

VarInfo* lookupVar(std::vector<std::unordered_map<std::string, VarInfo>>& scopes, const std::string& name)
{
    for (std::size_t i = scopes.size(); i-- > 0;) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) {
            return &it->second;
        }
    }
    return nullptr;
}

} // namespace

void UseBeforeInitRule::apply(const FileContext& file, const Config&, AnalysisResult& result) const
{
    const auto fns = findFunctions(file.tokens);

    for (const auto& fn : fns) {
        std::vector<std::unordered_map<std::string, VarInfo>> scopes;
        scopes.emplace_back();

        for (std::size_t i = fn.bodyStart; i < fn.bodyEnd && i < file.tokens.size(); ++i) {
            const auto& tok = file.tokens[i];

            if (tok.lexeme == "{") {
                scopes.emplace_back();
                continue;
            }
            if (tok.lexeme == "}") {
                if (scopes.size() > 1) {
                    scopes.pop_back();
                }
                continue;
            }

            // Declarations
            std::vector<DeclVar> decls;
            std::size_t next = i;
            if (tryParseDecl(file.tokens, i, decls, &next)) {
                for (const auto& d : decls) {
                    scopes.back()[d.nameTok.lexeme] = VarInfo{d.initialized};
                }
                i = next - 1;
                continue;
            }

            // Assignment initializes
            if (tok.kind == TokenKind::Identifier && !isMemberOrScopeAccess(file.tokens, i) && i + 1 < file.tokens.size() &&
                isAssignmentOp(file.tokens[i + 1])) {
                auto* v = lookupVar(scopes, tok.lexeme);
                if (v) {
                    v->initialized = true;
                }
                continue;
            }

            // Usage check
            if (tok.kind == TokenKind::Identifier && !isMemberOrScopeAccess(file.tokens, i)) {
                // skip function calls: name(
                if (i + 1 < file.tokens.size() && file.tokens[i + 1].lexeme == "(") {
                    continue;
                }

                auto* v = lookupVar(scopes, tok.lexeme);
                if (!v) {
                    continue;
                }

                // LHS of assignment is handled above; also skip address-of initializations like &x?
                if (!v->initialized) {
                    result.addIssue(Issue{Severity::Error,
                        file.path,
                        tok.line,
                        tok.column,
                        id(),
                        "Переменная '" + tok.lexeme + "' используется до инициализации.",
                        "Присвойте начальное значение при объявлении или перед использованием."});
                }
            }
        }
    }
}
