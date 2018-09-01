cmake_minimum_required(VERSION 2.6.4)

# define a variable of project name
set( project_name MakefileMake)

# define a project name
project (${project_name} LANGUAGES CXX)

# define a variable SOURCE_FILES with file GLOB
file(GLOB SOURCE_FILES *.cpp *.h)

# define sources files of an executable
add_executable(${project_name} ${SOURCE_FILES})
