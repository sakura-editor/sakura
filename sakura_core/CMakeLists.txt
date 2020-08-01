cmake_minimum_required(VERSION 3.12)

# define a variable of project name
set(project_name sakura_core)

# define a project name
project (${project_name} LANGUAGES CXX)

# define variables by file GLOB
file(GLOB_RECURSE SOURCE_FILES *.cpp)
file(GLOB_RECURSE HEADER_FILES *.h)
file(GLOB_RECURSE RESOURCE_FILES *.rc)

# define sources files of an object library
add_library(${project_name} OBJECT EXCLUDE_FROM_ALL ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES})

# add preprocessor symbol definitions
target_compile_definitions(${project_name} PUBLIC _WIN32)

# set include directories
target_include_directories(${project_name}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
)

# link libraries
target_link_libraries(${project_name}
  PUBLIC comctl32
  PUBLIC Imm32
  PUBLIC mpr
  PUBLIC imagehlp
  PUBLIC Shlwapi
  PUBLIC winmm
  PUBLIC windowscodecs
  PUBLIC msimg32
  )

# custom target SourceCodeHash
add_custom_target(SourceCodeHash
  "${CMAKE_SOURCE_DIR}/sakura/githash.bat" ../sakura_core
    BYPRODUCTS githash.h
    COMMENT "generate githash.h for c++ code."
    COMMAND_EXPAND_LISTS
    SOURCES ../.git/HEAD
  )

# custom target FunccodeForCode
add_custom_target(FunccodeForCode
  HeaderMake -in=Funccode_x.hsrc -out=${CMAKE_CURRENT_BINARY_DIR}/Funccode_enum.h -mode=enum -enum=EFunctionCode > NUL
    DEPENDS HeaderMake
    BYPRODUCTS Funccode_enum.h
    COMMENT "generate funccode header for c++ code."
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    SOURCES Funccode_x.hsrc
  )

if (MINGW)
  # build pre-compiled header for GCC
  add_custom_target(PchForGcc
    "${CMAKE_CXX_COMPILER}"
	  $<1:-D$<JOIN:$<TARGET_PROPERTY:${project_name},COMPILE_DEFINITIONS>,$<SEMICOLON>-D>>
	  $<0:-DUNICODE$<SEMICOLON>-D_DEBUG$<SEMICOLON>-D_UNICODE$<SEMICOLON>-D_WIN32$<SEMICOLON>-D_WIN32_WINNT=_WIN32_WINNT_WIN7>
	  $<1:-I$<JOIN:$<TARGET_PROPERTY:${project_name},INCLUDE_DIRECTORIES>,$<SEMICOLON>-I>>
	  $<0:-I../sakura_core$<SEMICOLON>-Isakura_core>
      $<$<CONFIG:Debug>:-g>
      $<$<CONFIG:Release>:-O3>
	  $<TARGET_PROPERTY:${project_name},COMPILE_OPTIONS>
	  -std=c++17
	  -MD
	  -MT ${CMAKE_CURRENT_BINARY_DIR}/StdAfx.h.gch
	  -MF ${CMAKE_CURRENT_BINARY_DIR}/StdAfx.h.gch.d
	  -o ${CMAKE_CURRENT_BINARY_DIR}/StdAfx.h.gch
	  -c ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h
      DEPENDS SourceCodeHash FunccodeForCode
      BYPRODUCTS StdAfx.h.gch
      COMMENT "generate pre-compiled header for gcc."
      COMMAND_EXPAND_LISTS
	  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../
      SOURCES StdAfx.h
    )

  # add dependencies for build
  add_dependencies(${project_name} PchForGcc)
endif()

# custom target FunccodeForResource
add_custom_target(FunccodeForResource
  HeaderMake -in=Funccode_x.hsrc -out=${CMAKE_CURRENT_BINARY_DIR}/Funccode_define.h -mode=define > NUL
    DEPENDS HeaderMake
    BYPRODUCTS Funccode_define.h
    COMMENT "generate funccode header for resource script."
    COMMAND_EXPAND_LISTS
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    SOURCES Funccode_x.hsrc
  )

# add dependencies for build
add_dependencies(${project_name} SourceCodeHash FunccodeForCode FunccodeForResource)

# define sources files of an executable
add_executable(sakura WIN32 $<TARGET_OBJECTS:${project_name}>)

set_target_properties(sakura
  PROPERTIES
    LINK_FLAGS "-municode"
  )

target_link_libraries(sakura ${project_name})
