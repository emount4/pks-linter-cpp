#include "rules/MemoryLeakRule.h"

#include "SeverityUtil.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

struct FunctionDef {
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
            } else if (toks[closeParen].lexeme == ")" && --paren == 0) {
                break;
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
            } else if (toks[bodyClose].lexeme == "}" && --depth == 0) {
                break;
            }
        }
        if (bodyClose >= toks.size()) {
            continue;
        }

        out.push_back(FunctionDef{bodyOpen + 1, bodyClose});
        i = bodyClose;
    }
    return out;
}

bool isMemberOrScopeAccess(const std::vector<Token>& toks, std::size_t idx)
{
    if (idx == 0) {
        return false;
    }
    const auto& prev = toks[idx - 1].lexeme;
    return prev == "." || prev == "->" || prev == "::";
}

std::string findAssignedVar(const std::vector<Token>& toks, std::size_t newIdx)
{
    std::size_t i = newIdx;
    while (i > 0) {
        --i;
        if (toks[i].lexeme == ";" || toks[i].lexeme == "{" || toks[i].lexeme == "}") {
            break;
        }
        if (toks[i].lexeme == "=") {
            for (std::size_t j = i; j-- > 0;) {
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

struct DeleteExpr {
    std::string var;
    bool arrayDelete{false};
};

DeleteExpr parseDelete(const std::vector<Token>& toks, std::size_t deleteIdx)
{
    DeleteExpr expr;
    std::size_t i = deleteIdx + 1;
    if (i + 1 < toks.size() && toks[i].lexeme == "[" && toks[i + 1].lexeme == "]") {
        expr.arrayDelete = true;
        i += 2;
    }
    if (i < toks.size() && toks[i].kind == TokenKind::Identifier) {
        expr.var = toks[i].lexeme;
    }
    return expr;
}

bool isArrayNew(const std::vector<Token>& toks, std::size_t newIdx)
{
    for (std::size_t i = newIdx + 1; i < toks.size(); ++i) {
        if (toks[i].lexeme == ";") {
            return false;
        }
        if (toks[i].lexeme == "[") {
            return true;
        }
    }
    return false;
}

struct Allocation {
    Token site;
    bool arrayNew{false};
    bool released{false};
};

} 

void MemoryLeakRule::apply(const FileContext& file, const Config& config, AnalysisResult& result) const
{
    const auto severity = configuredSeverity(config, id());

    for (const auto& fn : findFunctions(file.tokens)) {
        std::unordered_map<std::string, Allocation> allocations;

        for (std::size_t i = fn.bodyStart; i < fn.bodyEnd && i < file.tokens.size(); ++i) {
            const auto& tok = file.tokens[i];
            if (tok.kind == TokenKind::Keyword && tok.lexeme == "new") {
                const auto var = findAssignedVar(file.tokens, i);
                if (var.empty()) {
                    result.addIssue(Issue{severity, file.path, tok.line, tok.column, id(),
                        "Память выделена через new, но результат не сохранен в отслеживаемый указатель.",
                        "Сохраните указатель и освободите память через delete/delete[] или используйте RAII."});
                } else {
                    allocations[var] = Allocation{tok, isArrayNew(file.tokens, i), false};
                }
                continue;
            }

            if (tok.kind == TokenKind::Keyword && tok.lexeme == "delete") {
                const auto expr = parseDelete(file.tokens, i);
                if (expr.var.empty()) {
                    continue;
                }

                auto it = allocations.find(expr.var);
                if (it == allocations.end()) {
                    result.addIssue(Issue{severity, file.path, tok.line, tok.column, id(),
                        "Указатель '" + expr.var + "' освобождается без соответствующего new в этой функции.",
                        "Сделайте владение памятью явным и освобождайте только указатели, выделенные в этой области."});
                    continue;
                }

                if (it->second.arrayNew != expr.arrayDelete) {
                    result.addIssue(Issue{severity, file.path, tok.line, tok.column, id(),
                        "Несоответствие выделения и освобождения памяти для '" + expr.var + "'.",
                        "Используйте delete[] для new[] и delete для одиночного new."});
                }
                it->second.released = true;
            }
        }

        for (const auto& [var, alloc] : allocations) {
            if (alloc.released) {
                continue;
            }
            result.addIssue(Issue{severity, file.path, alloc.site.line, alloc.site.column, id(),
                "Потенциальная утечка памяти: '" + var + "' выделена через new, но не освобождена в этой функции.",
                "Добавьте соответствующий delete/delete[] или замените ручное управление памятью на RAII."});
        }
    }
}
