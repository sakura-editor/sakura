cmake_minimum_required(VERSION 3.11)

# define a variable of project name
set( project_name HeaderMake)

# define a project name
project (${project_name})

# define a variable SOURCE_FILES with file GLOB
file(GLOB SOURCE_FILES *.cpp *.h)

# define sources files of an executable
add_executable(${project_name} ${SOURCE_FILES})

target_compile_definitions(${project_name} PUBLIC _CONSOLE UNICODE _UNICODE)

install(
    TARGETS HeaderMake
    EXPORT HeaderMake-targets
    RUNTIME DESTINATION bin
)
