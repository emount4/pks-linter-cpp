#include "rules/MemoryLeakRule.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

struct FunctionDef {
    std::string name;
    std::size_t bodyStart{0};
    std::size_t bodyEnd{0};
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

        std::size_t k = j + 1;
        while (k < toks.size() && (toks[k].lexeme == "const" || toks[k].lexeme == "noexcept")) {
            ++k;
        }
        if (k >= toks.size() || toks[k].lexeme != "{") {
            continue;
        }

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

        out.push_back(FunctionDef{toks[nameIdx].lexeme, k + 1, m});
        i = m;
    }

    return out;
}

bool isMemberOrScopeAccess(const std::vector<Token>& toks, std::size_t idx)
{
    if (idx == 0) {
        return false;
    }
    auto prev = toks[idx - 1].lexeme;
    return prev == "." || prev == "->" || prev == "::";
}

std::string findAssignedVar(const std::vector<Token>& toks, std::size_t newIdx)
{
    // Search backwards in statement for '='
    std::size_t i = newIdx;
    while (i > 0) {
        --i;
        const auto& t = toks[i];
        if (t.lexeme == ";" || t.lexeme == "{" || t.lexeme == "}") {
            break;
        }
        if (t.kind == TokenKind::Operator && t.lexeme == "=") {
            // find identifier on the left
            std::size_t j = i;
            while (j > 0) {
                --j;
                if (toks[j].lexeme == ";" || toks[j].lexeme == "{" || toks[j].lexeme == "}") {
                    break;
                }
                if (toks[j].kind == TokenKind::Identifier && !isMemberOrScopeAccess(toks, j)) {
                    return toks[j].lexeme;
                }
            }
            break;
        }
    }
    return {};
}

std::string parseDeleteVar(const std::vector<Token>& toks, std::size_t deleteIdx)
{
    std::size_t i = deleteIdx + 1;
    if (i < toks.size() && toks[i].lexeme == "[") {
        // delete[]
        while (i < toks.size() && toks[i].lexeme != "]") {
            ++i;
        }
        if (i < toks.size() && toks[i].lexeme == "]") {
            ++i;
        }
    }
    if (i < toks.size() && toks[i].kind == TokenKind::Identifier) {
        return toks[i].lexeme;
    }
    return {};
}

} // namespace

void MemoryLeakRule::apply(const FileContext& file, const Config&, AnalysisResult& result) const
{
    const auto fns = findFunctions(file.tokens);

    for (const auto& fn : fns) {
        std::unordered_set<std::string> allocated;
        std::unordered_map<std::string, Token> allocationSite;

        for (std::size_t i = fn.bodyStart; i < fn.bodyEnd && i < file.tokens.size(); ++i) {
            const auto& tok = file.tokens[i];

            if (tok.kind == TokenKind::Keyword && tok.lexeme == "new") {
                auto var = findAssignedVar(file.tokens, i);
                if (!var.empty()) {
                    allocated.insert(var);
                    allocationSite.try_emplace(var, tok);
                } else {
                    result.addIssue(Issue{Severity::Warning,
                        file.path,
                        tok.line,
                        tok.column,
                        id(),
                        "Выделение памяти через new без сохранения результата.",
                        "Сохраните указатель в переменную и освободите память через delete (или используйте RAII)."});
                }
                continue;
            }

            if (tok.kind == TokenKind::Keyword && tok.lexeme == "delete") {
                auto var = parseDeleteVar(file.tokens, i);
                if (!var.empty()) {
                    allocated.erase(var);
                    allocationSite.erase(var);
                }
                continue;
            }
        }

        for (const auto& var : allocated) {
            auto it = allocationSite.find(var);
            int line = 0;
            int col = 0;
            if (it != allocationSite.end()) {
                line = it->second.line;
                col = it->second.column;
            }
            result.addIssue(Issue{Severity::Warning,
                file.path,
                line,
                col,
                id(),
                "Потенциальная утечка памяти: '" + var + "' выделена через new, но не освобождена delete в пределах функции.",
                "Добавьте delete для освобождения или используйте умные указатели (std::unique_ptr)."});
        }
    }
}
