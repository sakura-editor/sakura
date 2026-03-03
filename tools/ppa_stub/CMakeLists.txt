cmake_minimum_required(VERSION 3.12)

# dllの出力先フォルダーを決める
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  if(CMAKE_GENERATOR MATCHES "^Visual Studio")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${CMAKE_GENERATOR_PLATFORM}/$<CONFIG>")
  else()
    if(NOT DEFINED BUILD_PLATFORM)
      if(DEFINED ENV{BUILD_PLATFORM})
        set(BUILD_PLATFORM "$ENV{BUILD_PLATFORM}")
      elseif(DEFINED ENV{MSYSTEM})
        set(BUILD_PLATFORM "$ENV{MSYSTEM}")
      else()
        set(BUILD_PLATFORM "out")
      endif()
    endif()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${BUILD_PLATFORM}/${CMAKE_BUILD_TYPE}")
  endif()
endif()

set(OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

# define a variable of project name
set(project_name ppa_stub)

# define a project name
project (${project_name} LANGUAGES CXX C)

# define variables by file GLOB
file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE DEF_FILES *.def)

# define sources files of an executable
add_library(${project_name} MODULE ${SOURCES} ${DEF_FILES})

# Set C++ standard for project
target_compile_features(${project_name} PUBLIC cxx_std_20)

# add definitions for project
target_compile_definitions(${project_name}
  PUBLIC
    BUILD_PPA
)

# fix the error 'Missing variable is: CMAKE_RC_CREATE_SHARED_LIBRARY'
# https://cmake.org/pipermail/cmake/2012-January/048647.html
set_target_properties(${project_name}
  PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
    LINKER_LANGUAGE "CXX"
    PDB_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
)

add_custom_command(TARGET ${project_name} PRE_LINK
  COMMAND ${CMAKE_COMMAND} -E remove -f $<TARGET_FILE:${project_name}>
)

if(MINGW)
  # avoid prefixing of DLL name, set PREFIX to blank.
  # https://cmake.org/cmake/help/v3.12/prop_tgt/PREFIX.html?highlight=prefix
  set_target_properties(${project_name}
    PROPERTIES
      PREFIX		""
  )
endif()
