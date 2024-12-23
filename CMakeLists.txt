cmake_minimum_required(VERSION 3.24.0...3.30.0 FATAL_ERROR)

project(tre LANGUAGES C CXX VERSION 0.0.1)

option(TRE_ENABLE_INSTALL "whether to enable the install rule" ON)

include(FetchContent)
include(cmake/add_shader.cmake)

find_package(tr REQUIRED)
find_package(tref REQUIRED)

add_library(tre STATIC)
target_compile_features(tre PRIVATE cxx_std_20)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(tre PRIVATE -Wall -Wpedantic)
    target_compile_options(tre PRIVATE -fno-trapping-math -fno-math-errno -fno-signed-zeros -fassociative-math -ffp-contract=fast)
    target_compile_options(tre PRIVATE -march=x86-64-v2)
endif()
target_link_libraries(tre PUBLIC tref::tref tr::tr)
set_target_properties(tre PROPERTIES DEBUG_POSTFIX "d")

add_shader(tre resources/renderer_2d.vert RENDERER_2D_VERT_SPV)
add_shader(tre resources/renderer_2d.frag RENDERER_2D_FRAG_SPV)
add_shader(tre resources/debug_text.vert DEBUG_TEXT_VERT_SPV)
add_shader(tre resources/debug_text.frag DEBUG_TEXT_FRAG_SPV)
add_embedded_file(tre resources/debug_text_font.bmp DEBUG_TEXT_FONT_BMP)
target_sources(tre PRIVATE
    src/atlas.cpp src/audio.cpp src/bitmap_text_renderer.cpp src/debug_text_renderer.cpp src/dynamic_text_renderer.cpp
    src/localization_manager.cpp src/renderer_2d.cpp src/render_view.cpp src/sampler.cpp src/state_manager.cpp
    src/static_text_manager.cpp src/text.cpp
)
target_sources(tre PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
    include/tre/atlas.hpp include/tre/audio.hpp include/tre/bitmap_text_renderer.hpp include/tre/debug_text_renderer.hpp
    include/tre/dynamic_text_renderer.hpp include/tre/localization_manager.hpp include/tre/renderer_2d.hpp include/tre/render_view.hpp
    include/tre/sampler.hpp include/tre/state_manager.hpp
    include/tre/static_text_manager.hpp include/tre/text.hpp include/tre/tre.hpp
)

if(TRE_ENABLE_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(TARGETS tre
        EXPORT treTargets
        FILE_SET HEADERS
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    install(
        EXPORT treTargets
        NAMESPACE tre::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/tre"
    )

    configure_package_config_file(
        cmake/treConfig.cmake.in
        "${PROJECT_BINARY_DIR}/treConfig.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/tre"
    )
    install(
        FILES "${PROJECT_BINARY_DIR}/treConfig.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/tre"
    )

    write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/treConfigVersion.cmake"
        COMPATIBILITY SameMinorVersion
    )
    install(
        FILES "${PROJECT_BINARY_DIR}/treConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/tre"
    )
endif()