cmake_minimum_required(VERSION 3.6)
include(${CMAKE_SOURCE_DIR}/runtime.cmake)

project (sakura-unittest)

option(BUILD_SHARED_LIBS "Build shared libraries (DLLs)." OFF)

# switch DLL or static libary by specifying by command line
set (LIB_TYPE STATIC)
if (BUILD_SHARED_LIBS)
  # User wants to build Dynamic Libraries, so change the LIB_TYPE variable to CMake keyword 'SHARED'
  set (LIB_TYPE SHARED)
endif (BUILD_SHARED_LIBS)

# set Target Directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(googletest)
add_subdirectory(unittests)

# turn on solution folder
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# create solution folder
set_target_properties(gmock       PROPERTIES FOLDER GoogleTest)
set_target_properties(gmock_main  PROPERTIES FOLDER GoogleTest)
set_target_properties(gtest       PROPERTIES FOLDER GoogleTest)
set_target_properties(gtest_main  PROPERTIES FOLDER GoogleTest)
