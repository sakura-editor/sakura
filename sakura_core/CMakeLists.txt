cmake_minimum_required(VERSION 3.0.2)

# define a variable of project name
set(project_name sakura)

# define a project name
project (${project_name})

# set the project as the startup project
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${project_name})

# define variables by file GLOB
file(GLOB_RECURSE SOURCE_FILES   ./*.cpp)
file(GLOB_RECURSE HEADER_FILES   ./*.h)
file(GLOB_RECURSE RESOURCE_FILES ./*.rc)

#foreach( srcTemp ${SOURCE_FILES})
#	message("SOURCE_FILES = " ${srcTemp})
#endforeach( srcTemp ${SOURCE_FILES})

# define sources files of an executable
add_executable(${project_name} WIN32 ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES})

# set pre-Build
add_custom_command(
	TARGET ${project_name}
	PRE_BUILD
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/update-gitHash.bat ${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(./)

# add definition
add_definitions(-DWIN32 -D_WINDOWS -DNOMINMAX -DWINVER=0x0500 -D_WIN32_WINNT=0x0500 -D_WIN32_IE=0x0501)

target_link_libraries(${project_name} comctl32.lib Imm32.lib mpr.lib imagehlp.lib Shlwapi.lib)
