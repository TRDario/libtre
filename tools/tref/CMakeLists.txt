cmake_minimum_required(VERSION 3.23.0...3.30.0 FATAL_ERROR)

project(tref LANGUAGES C CXX VERSION 1.0.0)

find_package(PkgConfig REQUIRED)
pkg_check_modules(lz4 REQUIRED IMPORTED_TARGET liblz4)

add_library(tref STATIC tref.cpp)
target_sources(tref PUBLIC FILE_SET HEADERS FILES tref.hpp)
target_compile_features(tref PUBLIC cxx_std_20)
target_link_libraries(tref PUBLIC PkgConfig::lz4 qoi)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/build/tref.pc.in ${CMAKE_CURRENT_BINARY_DIR}/tref.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tref.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/tref-config-version.cmake COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tref-config-version.cmake ${CMAKE_CURRENT_SOURCE_DIR}/build/tref-config.cmake DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/tref)
install(TARGETS tref EXPORT trefTargets FILE_SET HEADERS)
install(EXPORT trefTargets NAMESPACE tref:: DESTINATION ${CMAKE_INSTALL_DATADIR}/tref)