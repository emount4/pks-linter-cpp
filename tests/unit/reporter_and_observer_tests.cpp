#include <catch2/catch_test_macros.hpp>

#include "AnalyzerEngine.h"
#include "Reporter.h"
#include "ReporterObserver.h"
#include "IObserver.h"
#include "test_support.h"
#include "scenario_helpers.h"

#include <sstream>

TEST_CASE("Reporter sorts issues by file line and column")
{
    AnalysisResult result;
    result.filesChecked = 2;
    result.addIssue(Issue{Severity::Warning, "b.cpp", 3, 4, "R2", "second", "fix second"});
    result.addIssue(Issue{Severity::Warning, "a.cpp", 10, 1, "R1", "first", "fix first"});
    result.addIssue(Issue{Severity::Error, "a.cpp", 2, 7, "R0", "third", "fix third"});

    std::ostringstream out;
    Reporter::print(result, out);
    auto text = out.str();

    auto posThird = text.find("a.cpp:2:7");
    auto posFirst = text.find("a.cpp:10:1");
    auto posSecond = text.find("b.cpp:3:4");

    REQUIRE(posThird != std::string::npos);
    REQUIRE(posFirst != std::string::npos);
    REQUIRE(posSecond != std::string::npos);
    REQUIRE(posThird < posFirst);
    REQUIRE(posFirst < posSecond);
}

TEST_CASE("Reporter prints summary counts")
{
    AnalysisResult result;
    result.filesChecked = 5;
    result.addIssue(Issue{Severity::Warning, "a.cpp", 1, 1, "R1", "warn", "fix"});
    result.addIssue(Issue{Severity::Error, "a.cpp", 2, 1, "R2", "err", "fix"});

    std::ostringstream out;
    Reporter::print(result, out);
    auto text = out.str();

    REQUIRE(text.find("Files checked: 5") != std::string::npos);
    REQUIRE(text.find("Errors: 1") != std::string::npos);
    REQUIRE(text.find("Warnings: 1") != std::string::npos);
}

TEST_CASE("ReporterObserver emits the final report on analysis finished")
{
    AnalysisResult result;
    result.filesChecked = 1;
    result.addIssue(Issue{Severity::Warning, "x.cpp", 4, 2, "R", "message", "recommendation"});

    std::ostringstream out;
    ReporterObserver observer(out);
    observer.onAnalysisFinished(result);

    auto text = out.str();
    REQUIRE(text.find("[WARNING] x.cpp:4:2") != std::string::npos);
    REQUIRE(text.find("Files checked: 1") != std::string::npos);
}
