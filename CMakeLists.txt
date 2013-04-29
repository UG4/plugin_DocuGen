cmake_minimum_required(VERSION 2.6)

####
# ugdocu generator
####

project(P_UGDOCU)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/ug_includes.cmake")

set(srcUGDocu
	src/ugdocu.cpp
	src/completion_list_generation.cpp
	src/ugdocu_misc.cpp
	src/ug_docu_class_description.cpp
	src/class_hierarchy_provider.cpp
	src/html_generation.cpp
	src/cpp_generator.cpp
)

add_executable(ugdocu ${srcUGDocu})

target_link_libraries (ugdocu ug4)
