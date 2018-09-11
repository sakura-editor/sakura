cmake_minimum_required(VERSION 3.6)
include(${CMAKE_SOURCE_DIR}/runtime.cmake)

# define a variable of project name
set(project_name tests1)

# define a variable SRC with file GLOB
file(GLOB SRC ${CMAKE_CURRENT_LIST_DIR}/test*.cpp)

# add include directories for sakura_core
include_directories(${CMAKE_SOURCE_DIR}/../sakura_core)

# define sources files of an executable
add_executable(${project_name} ${SRC})

