cmake_minimum_required(VERSION 3.16)

project(stage10_variables LANGUAGES NONE)

set(MY_TOPIC "CMake variables")
option(ENABLE_DETAILS "Show detailed configuration messages" ON)
message(STATUS "Topic: ${MY_TOPIC}")

if(ENABLE_DETAILS)
    message(STATUS "Show details")
else()
    message(STATUS "Hide details")
endif()

option(BUILD_EXTRAS "Configure extra modules" ON)

if(BUILD_EXTRAS)
    add_subdirectory(extras)
endif()