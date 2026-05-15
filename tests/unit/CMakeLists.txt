add_executable(cpp_linter_unit_tests
    config_and_factory_tests.cpp
    tokenizer_tests.cpp
    file_scanner_tests.cpp
    rule_tests.cpp
    reporter_and_observer_tests.cpp
    analyzer_engine_tests.cpp
)

target_link_libraries(cpp_linter_unit_tests PRIVATE cpp_linter_core Catch2::Catch2WithMain)
target_include_directories(cpp_linter_unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(cpp_linter_unit_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/tests"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/tests"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/tests"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/tests"
)

add_test(NAME cpp_linter_unit_tests COMMAND cpp_linter_unit_tests)
