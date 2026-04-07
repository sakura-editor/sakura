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

set(project_name dll_plugin1)

project(${project_name} LANGUAGES CXX C)

file(GLOB_RECURSE SOURCES *.cpp)

add_library(${project_name} MODULE ${SOURCES})

target_compile_features(${project_name} PUBLIC cxx_std_20)

set_target_properties(${project_name}
  PROPERTIES
    OUTPUT_NAME "dll_plugin1"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
    LINKER_LANGUAGE "CXX"
    PDB_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
)

add_custom_command(TARGET ${project_name} PRE_LINK
  COMMAND ${CMAKE_COMMAND} -E remove -f $<TARGET_FILE:${project_name}>
)

if(MINGW)
  set_target_properties(${project_name}
    PROPERTIES
      PREFIX ""
  )
endif()
