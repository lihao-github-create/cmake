cmake_minimum_required(VERSION 3.16)

project(stage11_testing VERSION 1.0.0 LANGUAGES CXX)

add_library(calculator calculator.cpp)

target_include_directories(calculator
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(calculator PUBLIC cxx_std_17)

option(BUILD_TESTS "Build tests" ON)

enable_testing()

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

install(
    TARGETS calculator
    EXPORT CalculatorTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(
    FILES calculator.hpp
    DESTINATION include
)

install(
    EXPORT CalculatorTargets
    FILE CalculatorTargets.cmake
    NAMESPACE Calculator::
    DESTINATION lib/cmake/Calculator
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CalculatorConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CalculatorConfig.cmake"
    INSTALL_DESTINATION lib/cmake/Calculator
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/CalculatorConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/CalculatorConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/CalculatorConfigVersion.cmake"
    DESTINATION lib/cmake/Calculator
)
