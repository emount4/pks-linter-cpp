#include "ConfigLoader.h"
#include "scenario_support.h"

#include <filesystem>
#include <iostream>

int main()
{
    scenarioEnableUtf8Console();
    const auto configPath = scenarioRepoRoot() / "config.example.ini";
    auto loaded = ConfigLoader::loadFromFile(configPath);
    if (loaded.warning) {
        std::cerr << *loaded.warning << '\n';
        return 1;
    }

    auto temp = std::filesystem::temp_directory_path() / "cpp_linter_config_roundtrip.ini";
    std::string error;
    if (!ConfigLoader::saveToFile(loaded.config, temp, &error)) {
        std::cerr << error << '\n';
        return 1;
    }

    auto roundtrip = ConfigLoader::loadFromFile(temp);
    if (roundtrip.warning) {
        std::cerr << *roundtrip.warning << '\n';
        return 1;
    }

    std::cout << "Config roundtrip OK\n";
    std::cout << "Включенных правил: " << roundtrip.config.enabledRules.size() << '\n';
    return 0;
}
