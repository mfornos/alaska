########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8)
project(alaska)
enable_language(C)

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

########################################################################
# MLM dependency
########################################################################
find_library(
    MLM mlm
    HINTS "/usr/local/lib"
)
if(${MLM} STREQUAL "MLM-NOTFOUND")
    message(FATAL_ERROR "Couldn't find the 'mlm' library")
endif()
message(STATUS "Found 'mlm' at: ${MLM}")

########################################################################
# CZMQ dependency
########################################################################
find_library(
    CZMQ czmq
    HINTS "/usr/local/lib"
)
if(${CZMQ} STREQUAL "CZMQ-NOTFOUND")
    message(FATAL_ERROR "Couldn't find the 'czmq' library")
endif()
message(STATUS "Found 'czmq' at: ${CZMQ}")

########################################################################
# LUA dependency
########################################################################
find_library(
    LUA lua
    HINTS "/usr/local/lib"
)
if(${LUA} STREQUAL "LUA-NOTFOUND")
    message(FATAL_ERROR "Couldn't find the 'lua' library")
endif()
message(STATUS "Found 'lua' at: ${LUA}")

########################################################################
# Executables
########################################################################
file( GLOB_RECURSE ALASKA_SOURCES "${SOURCE_DIR}/src/*.c" )
file( GLOB_RECURSE ALASKA_HEADERS "${SOURCE_DIR}/src/*.h" )

set ( ALASKA_INCLUDE_DIRS "" )
foreach ( _headerFile ${ALASKA_HEADERS} )
    get_filename_component( _dir ${_headerFile} PATH )
    list ( APPEND ALASKA_INCLUDE_DIRS ${_dir} )
endforeach()
list( REMOVE_DUPLICATES ALASKA_INCLUDE_DIRS )

# Alaska
include_directories( ${ALASKA_INCLUDE_DIRS} )
add_executable ( alaska ${ALASKA_SOURCES} )
target_link_libraries( alaska ${MLM} ${CZMQ} ${LUA} )
set_target_properties(
    alaska
    PROPERTIES  RUNTIME_OUTPUT_DIRECTORY "${SOURCE_DIR}/bin"
)

# Tools
add_executable ( alas ${SOURCE_DIR}/tools/alas.c )
target_link_libraries( alas ${MLM} ${CZMQ} )
set_target_properties(
    alas
    PROPERTIES  RUNTIME_OUTPUT_DIRECTORY "${SOURCE_DIR}/bin"
)

