add_executable(scenario_config_roundtrip scenario_config_roundtrip.cpp)
add_executable(scenario_sample_analysis scenario_sample_analysis.cpp)
add_executable(scenario_observer_flow scenario_observer_flow.cpp)

target_link_libraries(scenario_config_roundtrip PRIVATE cpp_linter_core)
target_link_libraries(scenario_sample_analysis PRIVATE cpp_linter_core)
target_link_libraries(scenario_observer_flow PRIVATE cpp_linter_core)

target_include_directories(scenario_config_roundtrip PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(scenario_sample_analysis PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(scenario_observer_flow PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(scenario_config_roundtrip PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/scenarios"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/tests/scenarios"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/tests/scenarios"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/tests/scenarios"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/tests/scenarios"
)
set_target_properties(scenario_sample_analysis PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/scenarios"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/tests/scenarios"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/tests/scenarios"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/tests/scenarios"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/tests/scenarios"
)
set_target_properties(scenario_observer_flow PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/scenarios"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/tests/scenarios"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/tests/scenarios"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/tests/scenarios"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/tests/scenarios"
)

add_test(NAME scenario_config_roundtrip COMMAND scenario_config_roundtrip)
add_test(NAME scenario_sample_analysis COMMAND scenario_sample_analysis)
add_test(NAME scenario_observer_flow COMMAND scenario_observer_flow)
