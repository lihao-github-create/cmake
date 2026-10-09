cmake_minimum_required(VERSION 3.16)

project(multi_directory_demo LANGUAGES CXX)

add_subdirectory(libs/math)
add_subdirectory(app)