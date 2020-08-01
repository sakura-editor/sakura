cmake_minimum_required(VERSION 3.12)

# define a variable of project name
set(project_name HeaderMake)

# define a project name
project (${project_name} LANGUAGES CXX)

# define a variable SOURCE_FILES with file GLOB
file(GLOB SOURCE_FILES *.cpp *.h)

# define sources files of an executable
add_executable(${project_name} ${SOURCE_FILES})

# add preprocessor symbol definitions
target_compile_definitions(${project_name} PUBLIC _CONSOLE)
