cmake_minimum_required(VERSION 3.16)

project(interface_demo LANGUAGES CXX)

add_library(math_headers INTERFACE)

target_include_directories(math_headers
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(math_headers
    INTERFACE
        cxx_std_17
)

add_executable(interface_app
    app/main.cpp
)

target_link_libraries(interface_app
    PRIVATE
        math_headers
)