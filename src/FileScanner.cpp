#include "FileScanner.h"

#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace {

// Приводит строку к нижнему регистру для нечувствительного сравнения.
std::string toLower(std::string s)
{
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

// Нормализует путь в строку с прямыми слешами и нижним регистром.
std::string normalizedGeneric(const std::filesystem::path& path)
{
    return toLower(path.lexically_normal().generic_string());
}

// Добавляет стандартные директории, которые не нужно анализировать.
void addDefaultExcludes(std::unordered_set<std::string>& excluded)
{
    for (const auto& dir : {"build", ".git", "third_party", "cmake-build-debug", "cmake-build-release"}) {
        excluded.insert(dir);
    }
}

// Проверяет, совпадает ли путь или имя файла с одним из исключений.
bool matchesPathOrName(const std::filesystem::path& root, const std::filesystem::path& candidate,
    const std::unordered_set<std::string>& excluded)
{
    const auto name = toLower(candidate.filename().generic_string());
    if (excluded.find(name) != excluded.end()) {
        return true;
    }

    std::error_code ec;
    const auto rel = std::filesystem::relative(candidate, root, ec);
    if (!ec && excluded.find(normalizedGeneric(rel)) != excluded.end()) {
        return true;
    }

    const auto absolute = std::filesystem::absolute(candidate, ec);
    return !ec && excluded.find(normalizedGeneric(absolute)) != excluded.end();
}

} 

// Сканирует проект, фильтрует расширения и применяет исключения.
std::vector<std::filesystem::path> FileScanner::scan(const std::filesystem::path& root, const ScanOptions& options)
{
    std::vector<std::filesystem::path> files;
    std::error_code ec;

    if (!std::filesystem::exists(root, ec)) {
        if (options.warnings) {
            options.warnings->push_back("Путь проекта не существует: " + root.string());
        }
        return files;
    }
    if (!std::filesystem::is_directory(root, ec)) {
        if (options.warnings) {
            options.warnings->push_back("Путь проекта не является директорией: " + root.string());
        }
        return files;
    }

    std::unordered_set<std::string> exts;
    for (const auto& ext : options.extensions) {
        auto normalized = toLower(ext);
        if (!normalized.empty() && normalized.front() != '.') {
            normalized = "." + normalized;
        }
        exts.insert(std::move(normalized));
    }

    std::unordered_set<std::string> excludedDirs;
    addDefaultExcludes(excludedDirs);
    for (const auto& d : options.excludedDirs) {
        excludedDirs.insert(normalizedGeneric(d));
    }

    std::unordered_set<std::string> excludedFiles;
    for (const auto& f : options.excludedFiles) {
        excludedFiles.insert(normalizedGeneric(f));
    }

    std::filesystem::recursive_directory_iterator it(
        root, std::filesystem::directory_options::skip_permission_denied, ec);
    std::filesystem::recursive_directory_iterator end;
    if (ec) {
        if (options.warnings) {
            options.warnings->push_back("Не удалось просканировать директорию проекта: " + ec.message());
        }
        return files;
    }

    for (; it != end; it.increment(ec)) {
        if (ec) {
            if (options.warnings) {
                options.warnings->push_back("Недоступный элемент пропущен: " + ec.message());
            }
            ec.clear();
            continue;
        }

        const auto p = it->path();
        if (it->is_directory(ec)) {
            if (matchesPathOrName(root, p, excludedDirs)) {
                it.disable_recursion_pending();
            }
            continue;
        }
        if (ec) {
            ec.clear();
            continue;
        }

        if (!it->is_regular_file(ec)) {
            ec.clear();
            continue;
        }

        if (matchesPathOrName(root, p, excludedFiles)) {
            continue;
        }

        const auto ext = toLower(p.extension().string());
        if (!ext.empty() && exts.find(ext) != exts.end()) {
            files.push_back(p.lexically_normal());
        }
    }

    std::sort(files.begin(), files.end(), [](const auto& a, const auto& b) {
        return a.generic_string() < b.generic_string();
    });
    return files;
}
