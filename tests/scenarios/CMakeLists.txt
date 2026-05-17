set(SCENARIO_TARGETS
    scenario_config_roundtrip
    scenario_sample_analysis
    scenario_observer_flow
    scenario_interactive_config
    scenario_large_project
)

foreach(target IN LISTS SCENARIO_TARGETS)
    add_executable(${target} ${target}.cpp)
    target_link_libraries(${target} PRIVATE cpp_linter_core)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/scenarios"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/tests/scenarios"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/tests/scenarios"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/tests/scenarios"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/tests/scenarios"
    )
    add_test(NAME ${target} COMMAND ${target})
endforeach()
