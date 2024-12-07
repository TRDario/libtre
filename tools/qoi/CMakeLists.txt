cmake_minimum_required(VERSION 3.23.0...3.30.0 FATAL_ERROR)

project(qoi LANGUAGES C VERSION 1.0.0)
add_library(qoi STATIC qoi.c)
target_sources(qoi PUBLIC FILE_SET HEADERS FILES qoi.h)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/build/qoi.pc.in ${CMAKE_CURRENT_BINARY_DIR}/qoi.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qoi.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/qoi-config-version.cmake COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qoi-config-version.cmake ${CMAKE_CURRENT_SOURCE_DIR}/build/qoi-config.cmake DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/qoi)
install(TARGETS qoi EXPORT qoiTargets FILE_SET HEADERS)
install(EXPORT qoiTargets NAMESPACE qoi:: DESTINATION ${CMAKE_INSTALL_DATADIR}/qoi)