cmake_minimum_required(VERSION 2.8)
#libftdi as external project

#----------------------------------------------------------------------------
#
# This script provides the libftdi parts to the build process.
#
# 1) It depacks the archiv.
# 2) Eventually compile the sources.
# 3) It provides all necessary variables to link the library.
#
# The provided variables are:
#
#  LIBFTDI_LIBRARIES : The libftdi library with the complete path and all
#                     required system libraries. The list must be separated
#                     with whitespace.
#
#  LIBFTDI_DEFINES : All defines for the libftdi library. This should be a
#                   list of entries starting with "-D".
#
#  LIBFTDI_INCLUDE_DIRECTORIES : The include folders for the library headers.
#                               The list must be separated with whitespace.
#

PROJECT("libftdi")

INCLUDE(ExternalProject)


#----------------------------------------------------------------------------
#
# Build the project.
#

# Extend the C flags and CXX flags with '-fPIC' for linux 64bit or linking the
# static libftdi to a shared object will fail.
SET(SUB_C_FLAGS ${CMAKE_C_FLAGS})
SET(SUB_CXX_FLAGS ${CMAKE_CXX_FLAGS})
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"  AND  "${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	LIST(APPEND SUB_C_FLAGS "-fPIC")
	LIST(APPEND SUB_CXX_FLAGS "-fPIC")
ENDIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"  AND  "${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")

SET(LIBFTDI_CMAKE_ARGS "")
#list baut eine ganze zeile aus den sachen die mit append hinzugefügt wurden 
LIST(APPEND LIBFTDI_CMAKE_ARGS "-DCMAKE_C_FLAGS=${SUB_C_FLAGS}")
LIST(APPEND LIBFTDI_CMAKE_ARGS "-DCMAKE_CXX_FLAGS=${SUB_CXX_FLAGS}")
LIST(APPEND LIBFTDI_CMAKE_ARGS "-DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")
LIST(APPEND LIBFTDI_CMAKE_ARGS "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
LIST(APPEND LIBFTDI_CMAKE_ARGS "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
LIST(APPEND LIBFTDI_CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/libftdi1-1.2")
LIST(APPEND LIBFTDI_CMAKE_ARGS "-DCMAKE_SHARED_LINKER_FLAGS=${CMAKE_SHARED_LINKER_FLAGS}")
LIST(APPEND LIBFTDI_CMAKE_ARGS "-DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}")
LIST(APPEND LIBFTDI_CMAKE_ARGS "-DLIBUSB_INCLUDE_DIR=${LIBUSB_INCLUDE_DIRECTORIES}")
LIST(APPEND LIBFTDI_CMAKE_ARGS "-DLIBUSB_LIBRARIES=${LIBUSB_LIBRARY}")
# Do not build the libftdi examples.
LIST(APPEND LIBFTDI_CMAKE_ARGS "-DEXAMPLES=OFF")
# Do not build the Python bindings.
LIST(APPEND LIBFTDI_CMAKE_ARGS "-DPYTHON_BINDINGS=OFF")
LIST(APPEND LIBFTDI_CMAKE_ARGS "-DLINK_PYTHON_LIBRARY=OFF")
# Do not build the eeprom tool.
LIST(APPEND LIBFTDI_CMAKE_ARGS "-DFTDI_EEPROM=OFF")


IF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	LIST(APPEND LIBFTDI_CMAKE_ARGS "-DCMAKE_RC_COMPILER=${CMAKE_RC_COMPILER}")
ENDIF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")



ExternalProject_Add(TARGET_libftdi
                    # The target needs libusb.
                    DEPENDS TARGET_libusb
                    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libftdi1-1.2
                    # Use the source archive in the current folder.
                    URL ${CMAKE_CURRENT_SOURCE_DIR}/libftdi1-1.2.tar.bz2
                    URL_HASH SHA1=636b9fcc745bf4eeb027a1dd891f770158444f7a
                    # Add all of the custom CMAKE arguments.
                    CMAKE_ARGS ${LIBFTDI_CMAKE_ARGS}
                    INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install
)


# 3) Provide all variables for the build.
SET(LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/libftdi1-1.2/lib/libftdi1.a)
SET(INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/libftdi1-1.2/include/libftdi1)


IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"  AND  "${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	# SET(LDFLAGS "-pthread -lrt -ludev")
ENDIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"  AND  "${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")


# ----------------------------------------------------------------------------

# Store all information about the library in the cache.

SET(LIBFTDI_LIBRARY              ${LIBRARY}              CACHE INTERNAL "The libftdi library with the complete path.")
SET(LIBFTDI_LDFLAGS              ${LDFLAGS}              CACHE INTERNAL "All linker flags required to link the library. The list must be separated with whitespace.")
SET(LIBFTDI_DEFINES              ${DEFINES}              CACHE INTERNAL "All defines for the libftdi library. This should be a list of entries starting with -D.")
SET(LIBFTDI_INCLUDE_DIRECTORIES  ${INCLUDE_DIRECTORIES}  CACHE INTERNAL "The include folders for the library headers. The list must be separated with whitespace.")
