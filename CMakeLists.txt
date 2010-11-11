cmake_minimum_required(VERSION 2.6)

####
# ugdocu generator
####

project(P_UGDOCU)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include("../../ug_cmake_includes.txt")

set(srcUGDocu	    src/main.cpp)

target_link_libraries (ugdocu ug)

add_executable(ugdocu ${srcUGDocu})
