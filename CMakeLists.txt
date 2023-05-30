cmake_minimum_required(VERSION 3.10)
project(Sep4IotDevice CXX C)
set(project_name Sep4IotDevice)

# Included for debugging
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g --coverage")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g --coverage")

add_subdirectory(tests)