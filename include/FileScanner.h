#pragma once

#include <filesystem>
#include <string>
#include <vector>

class FileScanner {
public:
    struct ScanOptions {
        std::vector<std::string> extensions;
        std::vector<std::string> excludedDirs;
        std::vector<std::string> excludedFiles;
        std::vector<std::string>* warnings{nullptr};
    };

    // Рекурсивно сканирует проект и возвращает подходящие исходные файлы.
    static std::vector<std::filesystem::path> scan(const std::filesystem::path& root, const ScanOptions& options);
};
