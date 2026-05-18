#include <catch2/catch_test_macros.hpp>

#include "FileScanner.h"
#include "test_support.h"

#include <algorithm>

// Проверяет фильтрацию файлов по поддерживаемым расширениям.
TEST_CASE("FileScanner returns only files with matching extensions")
{
    auto root = makeTempDir("scanner_ext");
    writeTextFile(root / "a.cpp", "int a = 1;\n");
    writeTextFile(root / "b.h", "#pragma once\n");
    writeTextFile(root / "c.txt", "ignored\n");

    FileScanner::ScanOptions options;
    options.extensions = {".cpp", ".h"};

    auto files = FileScanner::scan(root, options);
    REQUIRE(files.size() == 2);
    REQUIRE(files[0].filename().string() == "a.cpp");
    REQUIRE(files[1].filename().string() == "b.h");
}

// Проверяет исключение директорий по имени.
TEST_CASE("FileScanner excludes directories by name")
{
    auto root = makeTempDir("scanner_excluded");
    writeTextFile(root / "src" / "main.cpp", "int main() {}\n");
    writeTextFile(root / "build" / "generated.cpp", "int x = 0;\n");

    FileScanner::ScanOptions options;
    options.extensions = {".cpp"};
    options.excludedDirs = {"build"};

    auto files = FileScanner::scan(root, options);
    REQUIRE(files.size() == 1);
    REQUIRE(files[0].filename().string() == "main.cpp");
}

// Проверяет, что расширения файлов сравниваются без учета регистра.
TEST_CASE("FileScanner matches extensions case-insensitively")
{
    auto root = makeTempDir("scanner_case_ext");
    writeTextFile(root / "A.CPP", "int a = 1;\n");
    writeTextFile(root / "B.HPP", "#pragma once\n");

    FileScanner::ScanOptions options;
    options.extensions = {".cpp", ".hpp"};

    auto files = FileScanner::scan(root, options);
    REQUIRE(files.size() == 2);
}

// Проверяет, что имена исключаемых директорий сравниваются без учета регистра.
TEST_CASE("FileScanner excludes directories case-insensitively")
{
    auto root = makeTempDir("scanner_case_dir");
    writeTextFile(root / "BUILD" / "skip.cpp", "int skip = 0;\n");
    writeTextFile(root / "src" / "keep.cpp", "int keep = 1;\n");

    FileScanner::ScanOptions options;
    options.extensions = {".cpp"};
    options.excludedDirs = {"build"};

    auto files = FileScanner::scan(root, options);
    REQUIRE(files.size() == 1);
    REQUIRE(files[0].filename().string() == "keep.cpp");
}

// Проверяет стабильную сортировку найденных файлов.
TEST_CASE("FileScanner sorts results deterministically")
{
    auto root = makeTempDir("scanner_sort");
    writeTextFile(root / "z.cpp", "int z = 0;\n");
    writeTextFile(root / "a.cpp", "int a = 0;\n");
    writeTextFile(root / "m.cpp", "int m = 0;\n");

    FileScanner::ScanOptions options;
    options.extensions = {".cpp"};

    auto files = FileScanner::scan(root, options);
    REQUIRE(files.size() == 3);
    REQUIRE(files[0].filename().string() == "a.cpp");
    REQUIRE(files[1].filename().string() == "m.cpp");
    REQUIRE(files[2].filename().string() == "z.cpp");
}
