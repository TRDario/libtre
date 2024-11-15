cmake_minimum_required(VERSION 3.24.0 FATAL_ERROR)
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

include(CMakePackageConfigHelpers)
include(FetchContent)
include(GNUInstallDirs)
include(build/compile_shader_spv.cmake)

project(tre LANGUAGES C CXX VERSION 0.0.1)
add_library(tre STATIC)

target_compile_features(tre PRIVATE cxx_std_20)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(tre PRIVATE -Wall -Wpedantic)
    target_compile_options(tre PRIVATE -fno-trapping-math -fno-math-errno -fno-signed-zeros -fassociative-math -ffp-contract=fast)
    target_compile_options(tre PRIVATE -march=x86-64-v2)
endif()

FetchContent_Declare(tr GIT_REPOSITORY https://github.com/TRDario/libtr.git GIT_TAG main FIND_PACKAGE_ARGS NAMES tr)
FetchContent_MakeAvailable(tr)
target_link_libraries(tre PRIVATE tr)

compile_shader_spv(${CMAKE_CURRENT_SOURCE_DIR}/resources/renderer_2d.vert RENDERER_2D_VERT_SPV)
compile_shader_spv(${CMAKE_CURRENT_SOURCE_DIR}/resources/renderer_2d.frag RENDERER_2D_FRAG_SPV)
target_sources(tre PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/renderer_2d.vert.spv.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/renderer_2d.frag.spv.hpp
)

target_sources(tre PRIVATE
    src/atlas.cpp src/localization_manager.cpp src/renderer_2d.cpp src/renderer_base.cpp src/state_manager.cpp
)
target_sources(tre PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
    include/tre/atlas.hpp include/tre/localization_manager.hpp include/tre/renderer_2d.hpp include/tre/renderer_base.hpp
    include/tre/state_manager.hpp
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/build/tre.pc.in ${CMAKE_CURRENT_BINARY_DIR}/tre.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tre.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/tre-config-version.cmake COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tre-config-version.cmake ${CMAKE_CURRENT_SOURCE_DIR}/build/tre-config.cmake DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/tre)
install(TARGETS tre EXPORT treTargets FILE_SET HEADERS)
install(EXPORT treTargets NAMESPACE tre:: DESTINATION ${CMAKE_INSTALL_DATADIR}/tre)