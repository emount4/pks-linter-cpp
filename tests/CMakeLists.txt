include(FetchContent)

find_package(Catch2 3 QUIET)
if (NOT Catch2_FOUND)
    FetchContent_Declare(
        Catch2
        URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.5.4.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(Catch2)
endif()

add_subdirectory(unit)
add_subdirectory(scenarios)
