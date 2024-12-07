cmake_minimum_required(VERSION 3.23.0...3.30.0 FATAL_ERROR)

project(stb_image LANGUAGES C VERSION 2.30.0)
add_library(stb_image STATIC stb_image.c)
target_sources(stb_image PUBLIC FILE_SET HEADERS FILES stb_image.h)