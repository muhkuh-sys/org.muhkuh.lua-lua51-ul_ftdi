cmake_minimum_required(VERSION 2.8)
#libusb cmakelists for the external project for led_analyzer project

#----------------------------------------------------------------------------
#
# This script provides the libusb parts to the build process.
#
# 1) It depacks the archiv.
# 2) Eventually compile the sources.
# 3) It provides all necessary variables to link the library.
#
# The provided variables are:
#
#  LIBUSB_LIBRARIES : The libusb library with the complete path and all
#                     required system libraries. The list must be separated
#                     with whitespace.
#
#  LIBUSB_DEFINES : All defines for the libusb library. This should be a
#                   list of entries starting with "-D".
#
#  LIBUSB_INCLUDE_DIRECTORIES : The include folders for the library headers.
#                               The list must be separated with whitespace.
#

PROJECT("libusb")

INCLUDE(ExternalProject)


#----------------------------------------------------------------------------
#
# Build the project.
#

SET(LIBUSB_CMAKE_ARGS "")
LIST(APPEND LIBUSB_CMAKE_ARGS "-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}")
LIST(APPEND LIBUSB_CMAKE_ARGS "-DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")
LIST(APPEND LIBUSB_CMAKE_ARGS "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
LIST(APPEND LIBUSB_CMAKE_ARGS "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
LIST(APPEND LIBUSB_CMAKE_ARGS "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")
LIST(APPEND LIBUSB_CMAKE_ARGS "-DCMAKE_SHARED_LINKER_FLAGS=${CMAKE_SHARED_LINKER_FLAGS}")
LIST(APPEND LIBUSB_CMAKE_ARGS "-DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}")




IF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	LIST(APPEND LIBUSB_CMAKE_ARGS "-DCMAKE_RC_COMPILER=${CMAKE_RC_COMPILER}")
ENDIF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

ExternalProject_Add(TARGET_libusb
                    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libusb-1.0.20
                    URL ${CMAKE_CURRENT_SOURCE_DIR}/libusb-1.0.20.tar.bz2
                    PATCH_COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/templates ${CMAKE_CURRENT_BINARY_DIR}/libusb-1.0.20/src/TARGET_libusb
                    CMAKE_ARGS ${LIBUSB_CMAKE_ARGS}
                    INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install DESTDIR=${CMAKE_CURRENT_BINARY_DIR}/libusb-1.0.20
					#annahme: mingw oder linux target - im toplevel make.txt das korrekte make kommando ermitteln 
					# cmake current make executable 
)


# 3) Provide all variables for the build.
SET(LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/libusb-1.0.20/install/lib/libusb.a)
SET(INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/libusb-1.0.20/install/include/libusb-1.0)


IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"  AND  "${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	SET(LDFLAGS "-pthread;-lrt;-ludev")
ENDIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"  AND  "${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")

SET(DEFINES -DROMLOADER_USB_LIBUSB_VERSION=1)






#SET(LIBUSB_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/../libusb/lib/libusb-1.0.19/src/TARGET_libusb/libusb)
#SET(LIBUSB_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/../libusb/lib/libusb-1.0.19/src/TARGET_libusb-build/libusb.a)


# ----------------------------------------------------------------------------

# Store all information about the library in the cache.

SET(LIBUSB_LIBRARY              ${LIBRARY}              CACHE INTERNAL "The libusb library with the complete path.")
SET(LIBUSB_LDFLAGS              ${LDFLAGS}              CACHE INTERNAL "All linker flags required to link the library. The list must be separated with whitespace.")
SET(LIBUSB_DEFINES              ${DEFINES}              CACHE INTERNAL "All defines for the libusb library. This should be a list of entries starting with -D.")
SET(LIBUSB_INCLUDE_DIRECTORIES  ${INCLUDE_DIRECTORIES}  CACHE INTERNAL "The include folders for the library headers. The list must be separated with whitespace.")
