cmake_minimum_required(VERSION 3.16)

project(library_demo LANGUAGES CXX)

add_library(calculator
    src/calculator.cpp
)

target_include_directories(calculator
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(calculator
    PUBLIC
        cxx_std_17
)

add_executable(app
    app/main.cpp
)

target_link_libraries(app
    PRIVATE
        calculator
)