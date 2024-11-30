cmake_minimum_required(VERSION 3.1.0...3.30.0 FATAL_ERROR)

if (WIN32)
    option(ANSI_COLORED_TEXT "Enable the use of ANSI escape sequences for colored text output" OFF)
else ()
    option(ANSI_COLORED_TEXT "Enable the use of ANSI escape sequences for colored text output" ON)
endif ()

project(trefc LANGUAGES C CXX VERSION 0.0.1)
add_executable(trefc src/dependencies.cpp src/input.cpp src/image.cpp src/output.cpp src/main.cpp)
target_compile_features(trefc PRIVATE cxx_std_20)
if (ANSI_COLORED_TEXT)
    target_compile_definitions(trefc PRIVATE TREFC_ANSI_COLORS)
endif ()