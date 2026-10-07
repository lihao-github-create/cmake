cmake_minimum_required(VERSION 3.16)

project(target_demo LANGUAGES CXX)

add_executable(target_demo
    src/main.cpp
    src/math.cpp
)

target_include_directories(target_demo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(target_demo
    PRIVATE
        cxx_std_17
)