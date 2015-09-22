cmake_minimum_required(VERSION 3.0)

PROJECT("ul_ftdi")


#----------------------------------------------------------------------------
#
# Set custom definitions for several compiler.
#

# Microsoft: Do not show msvs security warnings about unsafe functions.
if(${CMAKE_BUILD_TOOL} MATCHES "(msdev|VCExpress|devenv|nmake)")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
ENDIF(${CMAKE_BUILD_TOOL} MATCHES "(msdev|VCExpress|devenv|nmake)")


# Microsoft: Set warning level to 2.
if(${CMAKE_BUILD_TOOL} MATCHES "(msdev|VCExpress|devenv)")
	add_definitions(/W2)
ENDIF(${CMAKE_BUILD_TOOL} MATCHES "(msdev|VCExpress|devenv)")


#----------------------------------------------------------------------------
#
# Include CMake scripts.

# Include the location of the dependencies.
INCLUDE(ivy/dependencies/ant_paths.cmake)

#include the version 
#INCLUDE(cmake/versions.cmake)

# Select the LUA version.
INCLUDE(cmake/muhkuh_lua.cmake)


#----------------------------------------------------------------------------
#
# Include the projects in the subdirectories.

add_subdirectory(external)
add_subdirectory(ul_ftdi)

