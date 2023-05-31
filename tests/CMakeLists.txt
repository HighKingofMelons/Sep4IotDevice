include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()

set(OBJECT_DIR ${CMAKE_SOURCE_DIR}/build/tests/CMakeFiles/Sep4IotDeviceTests.dir/__/src/)

# Source files to be tested
set(SOURCES_TEST
    fakes.c
    ../src/co2.c
    ../src/error.c
    ../src/temperature.c
    ../src/humidity.c
    ../src/actuation.c
    ../src/LoRaWANHandler.c
    ../src/handler_controller.c
)

set(GCOV_FILES
    ${OBJECT_DIR}actuation.c.gcda
    ${OBJECT_DIR}co2.c.gcda
    ${OBJECT_DIR}error.c.gcda
    ${OBJECT_DIR}humidity.c.gcda
    ${OBJECT_DIR}temperature.c.gcda
    ${OBJECT_DIR}utils.c.gcda
)

# The tests themselves
set(TESTS
    test_co2.cpp
    test_temperature.cpp
    error.cpp
    test_humidity.cpp
    actuation.cpp
    test_lorawan_handler.cpp
    test_handler_controller.cpp
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

add_custom_target(init
    COMMAND ${CMAKE_MAKE_PROGRAM} clean
    COMMAND rm -f ${OBJECT_DIR}/*.gcno
    COMMAND rm -f ${OBJECT_DIR}/*.gcda
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Create the gcov target. Run coverage tests with 'make coverage'

add_custom_target(coverage
    COMMAND mkdir -p gcoverage
    COMMAND ${CMAKE_MAKE_PROGRAM} ${project_name}Tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_command(TARGET coverage
    COMMAND echo "=================== GCOV ===================="
    COMMAND gcov ${GCOV_FILES}
    COMMAND echo "-- Source diretorie: ${CMAKE_SOURCE_DIR}/src/"
    COMMAND echo "-- Coverage files have been output to ${CMAKE_BINARY_DIR}/gcoverage"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/gcoverage
    COMMAND mkdir -p lcoverage
    COMMAND echo "=================== LCOV ===================="
    COMMAND echo "-- Passing lcov tool under code coverage"
    COMMAND lcov --capture --directory ../ --output-file lcoverage/main_coverage.info
    COMMAND echo "-- Generating HTML output files"
    COMMAND genhtml lcoverage/main_coverage.info --output-directory lcoverage
)


# Create the gcov target. Run coverage tests with 'make gcov'
add_custom_target(gcov
    COMMAND mkdir -p gcoverage
    COMMAND ${CMAKE_MAKE_PROGRAM} ${project_name}Tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_command(TARGET gcov
    COMMAND echo "=================== GCOV ===================="
    COMMAND gcov ${GCOV_FILES}
    COMMAND echo "-- Source diretorie: ${CMAKE_SOURCE_DIR}/src/"
    COMMAND echo "-- Coverage files have been output to ${CMAKE_BINARY_DIR}/gcoverage"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/gcoverage
)

# Create the lcov target. Run result tests with 'make lcov'
add_custom_target(lcov
    COMMAND mkdir -p lcoverage
)

add_custom_command(TARGET lcov
    COMMAND echo "=================== LCOV ===================="
    COMMAND echo "-- Passing lcov tool under code coverage"
    COMMAND lcov --capture --directory ../ --output-file lcoverage/main_coverage.info
    COMMAND echo "-- Generating HTML output files"
    COMMAND genhtml lcoverage/main_coverage.info --output-directory lcoverage
)

include(GoogleTest)
gtest_discover_tests(${project_name}Tests)
