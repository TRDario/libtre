cmake_minimum_required(VERSION 3.23.0 FATAL_ERROR)
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

project(tre LANGUAGES C CXX VERSION 0.0.1)

add_library(tre STATIC)
target_compile_features(tre PRIVATE cxx_std_20)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(tre PRIVATE -Wall -Wpedantic)
    target_compile_options(tre PRIVATE -fno-trapping-math -fno-math-errno -fno-signed-zeros -fassociative-math -ffp-contract=fast)
    target_compile_options(tre PRIVATE -march=x86-64-v2)
endif()
target_sources(tre PRIVATE
    src/localization_manager.cpp src/state_manager.cpp
)
target_sources(tre PUBLIC
    FILE_SET HEADERS
    BASE_DIRS include
    FILES
        include/tre/localization_manager.hpp include/tre/state_manager.hpp
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