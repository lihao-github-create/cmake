add_library(math
    src/math.cpp
)

target_include_directories(math
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(math
    PUBLIC
        cxx_std_17
)