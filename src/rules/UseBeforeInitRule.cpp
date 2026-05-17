#include "rules/UseBeforeInitRule.h"

#include "SeverityUtil.h"

#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

struct FunctionDef {
    std::size_t paramStart{0};
    std::size_t paramEnd{0};
    std::size_t bodyStart{0};
    std::size_t bodyEnd{0};
};

bool isControlKeyword(const Token& t)
{
    return t.kind == TokenKind::Keyword &&
        (t.lexeme == "if" || t.lexeme == "for" || t.lexeme == "while" || t.lexeme == "switch" || t.lexeme == "catch");
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
        if (braceDepth != 0 || toks[i].lexeme != "(" || i == 0) {
            continue;
        }

        std::size_t nameIdx = i;
        while (nameIdx > 0) {
            --nameIdx;
            if (toks[nameIdx].kind == TokenKind::Identifier) {
                break;
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

        int paren = 1;
        std::size_t closeParen = i + 1;
        for (; closeParen < toks.size(); ++closeParen) {
            if (toks[closeParen].lexeme == "(") {
                ++paren;
            } else if (toks[closeParen].lexeme == ")") {
                --paren;
                if (paren == 0) {
                    break;
                }
            }
        }
        if (closeParen >= toks.size()) {
            continue;
        }

        std::size_t bodyOpen = closeParen + 1;
        while (bodyOpen < toks.size() && (toks[bodyOpen].lexeme == "const" || toks[bodyOpen].lexeme == "noexcept")) {
            ++bodyOpen;
        }
        if (bodyOpen >= toks.size() || toks[bodyOpen].lexeme != "{") {
            continue;
        }

        int depth = 1;
        std::size_t bodyClose = bodyOpen + 1;
        for (; bodyClose < toks.size(); ++bodyClose) {
            if (toks[bodyClose].lexeme == "{") {
                ++depth;
            } else if (toks[bodyClose].lexeme == "}") {
                --depth;
                if (depth == 0) {
                    break;
                }
            }
        }
        if (bodyClose >= toks.size()) {
            continue;
        }

        out.push_back(FunctionDef{i + 1, closeParen, bodyOpen + 1, bodyClose});
        i = bodyClose;
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
        static const std::unordered_set<std::string> types{"void", "bool", "char", "short", "int", "long",
            "float", "double", "signed", "unsigned", "auto", "string"};
        return types.find(t.lexeme) != types.end();
    }
    return t.kind == TokenKind::Identifier && !t.lexeme.empty() &&
        std::isupper(static_cast<unsigned char>(t.lexeme.front())) != 0;
}

bool isMemberOrScopeAccess(const std::vector<Token>& toks, std::size_t idx)
{
    if (idx == 0) {
        return false;
    }
    const auto& prev = toks[idx - 1].lexeme;
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

void reportUseIfNeeded(const FileContext& file, const Config& config, AnalysisResult& result,
    std::vector<std::unordered_map<std::string, VarInfo>>& scopes, const Token& tok)
{
    auto* v = lookupVar(scopes, tok.lexeme);
    if (!v || v->initialized) {
        return;
    }
    result.addIssue(Issue{configuredSeverity(config, "BUG-USE-BEFORE-INIT"), file.path, tok.line, tok.column,
        "BUG-USE-BEFORE-INIT",
        "Переменная '" + tok.lexeme + "' используется до инициализации.",
        "Инициализируйте переменную при объявлении или присвойте значение до чтения."});
}

void checkRangeForReads(const FileContext& file, const Config& config, AnalysisResult& result,
    std::vector<std::unordered_map<std::string, VarInfo>>& scopes, const std::vector<Token>& toks,
    std::size_t first, std::size_t last)
{
    for (std::size_t i = first; i < last && i < toks.size(); ++i) {
        const auto& tok = toks[i];
        if (tok.kind != TokenKind::Identifier || isMemberOrScopeAccess(toks, i)) {
            continue;
        }
        if (i + 1 < toks.size() && toks[i + 1].lexeme == "(") {
            continue;
        }
        if (i + 1 < toks.size() && toks[i + 1].lexeme == "=") {
            continue;
        }
        reportUseIfNeeded(file, config, result, scopes, tok);
    }
}

bool tryParseDeclaration(const FileContext& file, const Config& config, AnalysisResult& result,
    std::vector<std::unordered_map<std::string, VarInfo>>& scopes, std::size_t idx, std::size_t* outNext)
{
    const auto& toks = file.tokens;
    std::size_t i = idx;
    while (i < toks.size() && isQualifier(toks[i])) {
        ++i;
    }
    if (i >= toks.size() || !isTypeToken(toks[i])) {
        return false;
    }
    ++i;

    bool foundAny = false;
    for (;;) {
        while (i < toks.size() && (toks[i].lexeme == "*" || toks[i].lexeme == "&")) {
            ++i;
        }
        if (i >= toks.size() || toks[i].kind != TokenKind::Identifier) {
            return false;
        }

        Token name = toks[i++];
        bool initialized = false;

        if (i < toks.size() && toks[i].lexeme == "[") {
            int depth = 0;
            while (i < toks.size()) {
                if (toks[i].lexeme == "[") {
                    ++depth;
                } else if (toks[i].lexeme == "]" && --depth == 0) {
                    ++i;
                    break;
                }
                ++i;
            }
        }

        if (i < toks.size() && (toks[i].lexeme == "=" || toks[i].lexeme == "{" || toks[i].lexeme == "(")) {
            initialized = true;
            const std::size_t initStart = i + 1;
            int paren = toks[i].lexeme == "(" ? 1 : 0;
            int brace = toks[i].lexeme == "{" ? 1 : 0;
            int bracket = 0;
            ++i;
            const std::size_t readStart = i;
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
            checkRangeForReads(file, config, result, scopes, toks, readStart, i);
            (void)initStart;
        }

        scopes.back()[name.lexeme] = VarInfo{initialized};
        foundAny = true;

        if (i >= toks.size()) {
            return false;
        }
        if (toks[i].lexeme == ",") {
            ++i;
            continue;
        }
        if (toks[i].lexeme == ";") {
            *outNext = i + 1;
            return foundAny;
        }
        return false;
    }
}

std::vector<Token> extractParameterNames(const std::vector<Token>& toks, std::size_t first, std::size_t last)
{
    std::vector<Token> params;
    std::size_t segmentStart = first;
    for (std::size_t i = first; i <= last && i < toks.size(); ++i) {
        if (i == last || toks[i].lexeme == ",") {
            for (std::size_t j = i; j-- > segmentStart;) {
                if (toks[j].kind == TokenKind::Identifier) {
                    params.push_back(toks[j]);
                    break;
                }
            }
            segmentStart = i + 1;
        }
    }
    return params;
}

bool isAssignmentOp(const Token& t)
{
    return t.kind == TokenKind::Operator &&
        (t.lexeme == "=" || t.lexeme == "+=" || t.lexeme == "-=" || t.lexeme == "*=" || t.lexeme == "/=" ||
            t.lexeme == "%=");
}

} // пространство имен

void UseBeforeInitRule::apply(const FileContext& file, const Config& config, AnalysisResult& result) const
{
    for (const auto& fn : findFunctions(file.tokens)) {
        std::vector<std::unordered_map<std::string, VarInfo>> scopes;
        scopes.emplace_back();
        for (const auto& param : extractParameterNames(file.tokens, fn.paramStart, fn.paramEnd)) {
            scopes.back()[param.lexeme] = VarInfo{true};
        }

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

            std::size_t next = i;
            if (tryParseDeclaration(file, config, result, scopes, i, &next)) {
                i = next - 1;
                continue;
            }

            if (tok.kind == TokenKind::Identifier && !isMemberOrScopeAccess(file.tokens, i) &&
                i + 1 < file.tokens.size() && isAssignmentOp(file.tokens[i + 1])) {
                if (auto* v = lookupVar(scopes, tok.lexeme)) {
                    if (file.tokens[i + 1].lexeme != "=") {
                        reportUseIfNeeded(file, config, result, scopes, tok);
                    }
                    v->initialized = true;
                }
                continue;
            }

            if (tok.kind == TokenKind::Identifier && !isMemberOrScopeAccess(file.tokens, i)) {
                if (i + 1 < file.tokens.size() && file.tokens[i + 1].lexeme == "(") {
                    continue;
                }
                reportUseIfNeeded(file, config, result, scopes, tok);
            }
        }
    }
}
