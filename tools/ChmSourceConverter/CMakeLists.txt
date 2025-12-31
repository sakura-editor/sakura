cmake_minimum_required(VERSION 3.10)

# define a project
project (ChmSourceConverter)

if(NOT DEFINED CMD_VSWHERE OR NOT EXISTS "${CMD_VSWHERE}")
  message(FATAL_ERROR "vswhere.exe is required.")
endif()

# Use vswhere to find MSBuild
execute_process(
  COMMAND ${CMD_VSWHERE} -find "MSBuild\\**\\Bin\\MSBuild.exe" -latest -requires Microsoft.Component.MSBuild
  OUTPUT_VARIABLE CMD_MSBUILD
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE VSWHERE_RESULT
)

if(NOT VSWHERE_RESULT EQUAL 0 OR NOT CMD_MSBUILD)
  message(FATAL_ERROR "msbuild.exe was not found via vswhere.")
endif()

message(STATUS "Found MSBuild: ${CMD_MSBUILD}")

set(OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CHM_SOURCE_CONVERTER "${OUTPUT_PATH}/ChmSourceConverter.exe")

# Build ChmSourceConverter
add_custom_command(
  OUTPUT "${CHM_SOURCE_CONVERTER}"
  COMMAND ${CMD_MSBUILD} "${CMAKE_SOURCE_DIR}/ChmSourceConverter.sln"
    "/p:Platform=Any CPU"
    /p:Configuration=Debug
    /p:BaseIntermediateOutputPath=${CMAKE_BINARY_DIR}/obj/
    /p:BaseOutputPath=${CMAKE_BINARY_DIR}/bin/
    /p:OutputPath=${OUTPUT_PATH}/
    /t:ChmSourceConverter
    /v:q
    /nologo
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  COMMENT "Building ChmSourceConverter"
  VERBATIM
)

add_custom_target(${PROJECT_NAME} ALL
  DEPENDS "${CHM_SOURCE_CONVERTER}"
)

# Install the generated executable
install(
  PROGRAMS "${CHM_SOURCE_CONVERTER}"
  DESTINATION bin
)
