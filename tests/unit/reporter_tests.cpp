#include <catch2/catch_test_macros.hpp>

#include "Reporter.h"

#include <sstream>

// Проверяет сообщение об отсутствии нарушений и стабильную итоговую сводку.
TEST_CASE("Reporter prints no-issues message and stable summary")
{
    AnalysisResult result;
    result.filesChecked = 3;
    std::ostringstream out;
    Reporter::print(result, out);
    auto text = out.str();
    REQUIRE(text.find("Нарушений не найдено.") != std::string::npos);
    REQUIRE(text.find("проверено файлов: 3") != std::string::npos);
    REQUIRE(text.find("всего нарушений: 0") != std::string::npos);
}
