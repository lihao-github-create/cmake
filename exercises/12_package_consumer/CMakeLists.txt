cmake_minimum_required(VERSION 3.16)

project(stage12_package_consumer LANGUAGES CXX)

find_package(Calculator 1.0 CONFIG REQUIRED)

add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE Calculator::calculator)
