cmake_minimum_required(VERSION 3.16)

project(cpp_linter LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(cpp_linter_core STATIC
    src/AnalyzerEngine.cpp
    src/RuleFactory.cpp
    src/ConfigLoader.cpp
    src/FileScanner.cpp
    src/InteractiveConfig.cpp
    src/Reporter.cpp
    src/Tokenizer.cpp
    src/rules/IndentationRule.cpp
    src/rules/LineLengthRule.cpp
    src/rules/SpacingRule.cpp
    src/rules/NamingRule.cpp
    src/rules/UseBeforeInitRule.cpp
    src/rules/MemoryLeakRule.cpp
)

target_include_directories(cpp_linter_core PUBLIC include)

add_executable(cpp_linter
    src/main.cpp
)

target_link_libraries(cpp_linter PRIVATE cpp_linter_core)

set_target_properties(cpp_linter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}"
)

if (MSVC)
    target_compile_options(cpp_linter_core PRIVATE /W4)
    target_compile_options(cpp_linter PRIVATE /W4)
else()
    target_compile_options(cpp_linter_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(cpp_linter PRIVATE -Wall -Wextra -Wpedantic)
endif()

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
