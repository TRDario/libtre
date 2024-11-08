cmake_minimum_required(VERSION 3.23.0 FATAL_ERROR)
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

function(compile_shader_spv FILE)
    add_custom_command(
        OUTPUT ${FILE}.spv
        COMMAND glslang -G -o ${FILE}.spv ${FILE}
        DEPENDS ${FILE}
        VERBATIM
    )
    add_custom_command(
        OUTPUT ${FILE}.spv.hpp
        COMMAND xxd -i -n RENDERER_2D_VERT_SPV ${FILE}.spv ${FILE}.spv.hpp
        DEPENDS ${FILE}.spv
        VERBATIM
    )
endfunction()

set(RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
compile_shader_spv(${RESOURCES_DIR}/renderer_2d.vert)
compile_shader_spv(${RESOURCES_DIR}/renderer_2d.frag)

project(tre LANGUAGES C CXX VERSION 0.0.1)

add_library(tre STATIC)
target_compile_features(tre PRIVATE cxx_std_20)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(tre PRIVATE -Wall -Wpedantic)
    target_compile_options(tre PRIVATE -fno-trapping-math -fno-math-errno -fno-signed-zeros -fassociative-math -ffp-contract=fast)
    target_compile_options(tre PRIVATE -march=x86-64-v2)
endif()
target_sources(tre PRIVATE
    src/atlas.cpp src/localization_manager.cpp src/renderer_base.cpp src/state_manager.cpp
)
target_sources(tre PRIVATE
    ${RESOURCES_DIR}/renderer_2d.vert.spv.hpp ${RESOURCES_DIR}/renderer_2d.frag.spv.hpp
)
target_sources(tre PUBLIC
    FILE_SET HEADERS
    BASE_DIRS include
    FILES
        include/tre/atlas.hpp include/tre/localization_manager.hpp include/tre/renderer_base.hpp
        include/tre/state_manager.hpp
)
set_target_properties(tre PROPERTIES DEBUG_POSTFIX d)

#include(GNUInstallDirs)
#include(CMakePackageConfigHelpers)

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/build/tr.pc.in ${CMAKE_CURRENT_BINARY_DIR}/tr.pc @ONLY)
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tr.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
#write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/tr-config-version.cmake COMPATIBILITY AnyNewerVersion)
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tr-config-version.cmake ${CMAKE_CURRENT_SOURCE_DIR}/build/tr-config.cmake DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/tr)
#install(TARGETS tr EXPORT trTargets FILE_SET HEADERS)
#install(EXPORT trTargets NAMESPACE tr:: DESTINATION ${CMAKE_INSTALL_DATADIR}/tr)