####
# ugdocu generator
####
set(pluginName	ugdocu)
set(SOURCES
		src/ugdocu.cpp
		src/completion_list_generation.cpp
		src/ugdocu_misc.cpp
		src/ug_docu_class_description.cpp
		src/class_hierarchy_provider.cpp
		src/html_generation.cpp
		src/cpp_generator.cpp
	)

################################################################################
# The code below doesn't have to be changed (usually)
################################################################################
cmake_minimum_required(VERSION 2.6)

project(UG_PLUGIN_${pluginName})

# include the definitions and dependencies for ug-plugins.
include(${UG_ROOT_CMAKE_PATH}/ug_plugin_includes.cmake)

if(buildEmbeddedPlugins)
	# add the sources to ug4's sources
	EXPORTSOURCES(${CMAKE_CURRENT_SOURCE_DIR} ${SOURCES})
else(buildEmbeddedPlugins)
	# create a shared library from the sources and link it against ug4.
	add_library(${pluginName} SHARED ${SOURCES})
	target_link_libraries (${pluginName} ug4)
endif(buildEmbeddedPlugins)
