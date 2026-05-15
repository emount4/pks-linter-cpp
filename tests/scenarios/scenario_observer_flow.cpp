#include "AnalyzerEngine.h"
#include "Config.h"
#include "ConfigManager.h"
#include "IObserver.h"
#include "scenario_support.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

class ConsoleObserver final : public IObserver {
public:
    void onIssue(const IssueEvent& event) override
    {
        std::cout << "ISSUE " << event.filePath << " " << event.issue.ruleId << '\n';
    }

    void onFileStart(const std::string& path) override
    {
        std::cout << "START " << path << '\n';
    }

    void onFileEnd(const std::string& path) override
    {
        std::cout << "END " << path << '\n';
    }
};

int main()
{
    scenarioEnableUtf8Console();
    Config config = ConfigManager::instance().get();
    config.enabledRules = {"BUG-USE-BEFORE-INIT"};
    ConfigManager::instance().set(config);

    AnalyzerEngine engine;
    ConsoleObserver observer;
    engine.addObserver(&observer);

    auto result = engine.analyzeProject(scenarioRepoRoot() / "sample_project", ConfigManager::instance().get());
    std::cout << "Finished: " << result.filesChecked << " files\n";
    return 0;
}
