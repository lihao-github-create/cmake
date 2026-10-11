cmake_minimum_required(VERSION 3.16)

project(external_library_demo LANGUAGES CXX)

find_package(Eigen3 REQUIRED CONFIG)

add_executable(app main.cpp)

target_compile_features(app PRIVATE cxx_std_17)

target_link_libraries(app PRIVATE Eigen3::Eigen)