#include "AnalyzerEngine.h"
#include "Config.h"
#include "ConfigManager.h"
#include "ReporterObserver.h"
#include "RuleFactory.h"
#include "scenario_support.h"

#include <filesystem>
#include <iostream>

int main()
{
    scenarioEnableUtf8Console();
    Config config = ConfigManager::instance().get();
    if (config.enabledRules.empty()) {
        config.enabledRules = RuleFactory::enabledRuleIdsFromFlags(config);
    }
    ConfigManager::instance().set(config);

    AnalyzerEngine engine;
    ReporterObserver observer(std::cout);
    engine.addObserver(&observer);

    auto result = engine.analyzeProject(scenarioRepoRoot() / "sample_project", ConfigManager::instance().get());
    std::cout << "Сценарий завершен, нарушений: " << result.issues.size() << '\n';
    return result.errorCount() > 0 ? 0 : 1;
}
