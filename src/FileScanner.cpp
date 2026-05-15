#include "FileScanner.h"

#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace {

std::string toLower(std::string s)
{
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

} // namespace

std::vector<std::filesystem::path> FileScanner::scan(const std::filesystem::path& root, const ScanOptions& options)
{
    std::unordered_set<std::string> exts;
    for (const auto& ext : options.extensions) {
        exts.insert(toLower(ext));
    }

    std::unordered_set<std::string> excluded;
    for (const auto& d : options.excludedDirs) {
        excluded.insert(toLower(d));
    }

    std::vector<std::filesystem::path> files;

    std::error_code ec;
    std::filesystem::recursive_directory_iterator it(root, std::filesystem::directory_options::skip_permission_denied, ec);
    std::filesystem::recursive_directory_iterator end;

    for (; it != end; it.increment(ec)) {
        if (ec) {
            ec.clear();
            continue;
        }

        const auto& p = it->path();
        if (it->is_directory()) {
            auto name = toLower(p.filename().string());
            if (excluded.find(name) != excluded.end()) {
                it.disable_recursion_pending();
            }
            continue;
        }

        if (!it->is_regular_file()) {
            continue;
        }

        auto ext = toLower(p.extension().string());
        if (!ext.empty() && exts.find(ext) != exts.end()) {
            files.push_back(p);
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}
