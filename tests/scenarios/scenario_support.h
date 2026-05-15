#pragma once

#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

inline std::filesystem::path scenarioExecutableDir()
{
#ifdef _WIN32
    wchar_t buffer[32768];
    const DWORD length = GetModuleFileNameW(nullptr, buffer, static_cast<DWORD>(std::size(buffer)));
    if (length != 0) {
        return std::filesystem::path(std::wstring(buffer, buffer + length)).parent_path();
    }
#endif
    return std::filesystem::current_path();
}

inline std::filesystem::path scenarioRepoRoot()
{
    auto dir = scenarioExecutableDir();
    for (int i = 0; i < 3 && !dir.empty(); ++i) {
        dir = dir.parent_path();
    }
    return dir;
}

inline void scenarioEnableUtf8Console()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}
