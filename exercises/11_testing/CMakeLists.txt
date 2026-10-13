cmake_minimum_required(VERSION 3.16)

project(stage11_testing LANGUAGES CXX)

add_library(calculator calculator.cpp)

target_include_directories(calculator
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(calculator PUBLIC cxx_std_17)

option(BUILD_TESTS "Build tests" ON)

enable_testing()

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()