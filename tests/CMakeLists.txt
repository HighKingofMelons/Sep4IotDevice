include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()

# Source files to be tested
set(SOURCES_TEST
    ../src/utils.c
    fakes.c
    ../src/temperature/temperature.c
)

# The tests themselves
set(TESTS
    utils.cpp
    test_temperature.cpp
)

include_directories(
    ../include
    Drivers
    FreeRTOS
    ./
)

add_executable(
    ${project_name}Tests
    ${SOURCES_TEST} ${TESTS}
)
target_link_libraries(
    ${project_name}Tests
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${project_name}Tests)
