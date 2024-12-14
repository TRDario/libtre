cmake_minimum_required(VERSION 3.24.0...3.30.0 FATAL_ERROR)

option(BUILD_TOOLS "Build tools for working with tre formats" OFF)

project(tre LANGUAGES C CXX VERSION 0.0.1)
add_library(tre STATIC)

include(CMakePackageConfigHelpers)
include(FetchContent)
include(GNUInstallDirs)
include(build/add_shader.cmake)

target_compile_features(tre PRIVATE cxx_std_20)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(tre PRIVATE -Wall -Wpedantic)
    target_compile_options(tre PRIVATE -fno-trapping-math -fno-math-errno -fno-signed-zeros -fassociative-math -ffp-contract=fast)
    target_compile_options(tre PRIVATE -march=x86-64-v2)
endif()

FetchContent_Declare(tr GIT_REPOSITORY https://github.com/TRDario/libtr.git GIT_TAG main FIND_PACKAGE_ARGS NAMES tr)
FetchContent_MakeAvailable(tr)
target_link_libraries(tre PUBLIC tr)

add_shader(tre resources/renderer_2d.vert RENDERER_2D_VERT_SPV)
add_shader(tre resources/renderer_2d.frag RENDERER_2D_FRAG_SPV)
add_shader(tre resources/debug_text.vert DEBUG_TEXT_VERT_SPV)
add_shader(tre resources/debug_text.frag DEBUG_TEXT_FRAG_SPV)
add_embedded_file(tre resources/debug_text_font.bmp DEBUG_TEXT_FONT_BMP)

target_sources(tre PRIVATE
    src/atlas.cpp src/audio.cpp src/bitmap_text_renderer.cpp src/debug_text_renderer.cpp src/dynamic_text_renderer.cpp
    src/localization_manager.cpp src/renderer_2d.cpp src/renderer_base.cpp src/sampler.cpp src/state_manager.cpp
    src/static_text_renderer.cpp src/text.cpp
)
target_sources(tre PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
    include/tre/atlas.hpp include/tre/audio.hpp include/tre/bitmap_text_renderer.hpp include/tre/debug_text_renderer.hpp
    include/tre/dynamic_text_renderer.hpp include/tre/localization_manager.hpp include/tre/renderer_2d.hpp
    include/tre/renderer_base.hpp include/tre/sampler.hpp include/tre/state_manager.hpp
    include/tre/static_text_renderer.hpp include/tre/text.hpp
)

if (BUILD_TOOLS)
    add_subdirectory(tools)
else ()
    add_subdirectory(tools/qoi)
    add_subdirectory(tools/tref)
endif ()
target_link_libraries(tre PUBLIC tref)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/build/tre.pc.in ${CMAKE_CURRENT_BINARY_DIR}/tre.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tre.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/tre-config-version.cmake COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tre-config-version.cmake ${CMAKE_CURRENT_SOURCE_DIR}/build/tre-config.cmake DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/tre)
install(TARGETS tre EXPORT treTargets FILE_SET HEADERS)
install(EXPORT treTargets NAMESPACE tre:: DESTINATION ${CMAKE_INSTALL_DATADIR}/tre)