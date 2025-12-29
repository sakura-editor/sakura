cmake_minimum_required(VERSION 3.16)

project(CompiledHtmlHelp)

# Find vswhere
find_program(CMD_VSWHERE vswhere.exe
  DOC "Visual Studio Locator"
)

if(NOT CMD_VSWHERE)
  message(FATAL_ERROR "vswhere.exe was not found.")
endif()

message(STATUS "Found vswhere: ${CMD_VSWHERE}")

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

# Find PowerShell
find_program(CMD_PWSH pwsh.exe
  PATHS
    "${CMAKE_CURRENT_SOURCE_DIR}/../tools/vcpkg/downloads/tools"
  PATH_SUFFIXES
    "powershell-core-*-windows"
  DOC "PowerShell Core"
)

if(NOT CMD_PWSH)
  message(FATAL_ERROR "pwsh.exe was not found.")
endif()

message(STATUS "Found PowerShell Core: ${CMD_PWSH}")

# Find HTML Help Compiler
find_program(CMD_HHC hhc.exe
  PATHS
    "C:/Program Files (x86)/HTML Help Workshop"
    "$ENV{ProgramFiles\(x86\)}/HTML Help Workshop"
  DOC "HTML Help Compiler"
)

if(NOT CMD_HHC)
  message(FATAL_ERROR "hhc.exe was not found. Please install HTML Help Workshop.")
endif()

message(STATUS "Found HTML Help Compiler: ${CMD_HHC}")

# Check system locale
cmake_language(CALL execute_process
  COMMAND ${CMD_PWSH} -NoProfile -Command "([System.Globalization.CultureInfo]::CurrentCulture.Name)"
  OUTPUT_VARIABLE SYSTEM_LOCALE
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "System locale: ${SYSTEM_LOCALE}")

# Find Locale Emulator (only if locale is not ja-JP)
if(NOT SYSTEM_LOCALE STREQUAL "ja-JP")
  find_program(CMD_LEPROC LEProc.exe
    DOC "Locale Emulator Processor"
  )

  if(CMD_LEPROC)
    message(STATUS "Found Locale Emulator: ${CMD_LEPROC}")
  else()
    message(FATAL_ERROR "Locale Emulator not found. Please install Locale Emulator.")
  endif()
else()
  message(STATUS "  Locale Emulator not required")
endif()

# Build ChmSourceConverter
set(TOOL_SLN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../tools/ChmSourceConverter/ChmSourceConverter.sln)
set(CHM_CONVERTER ${CMAKE_CURRENT_BINARY_DIR}/ChmSourceConverter/bin/Release/ChmSourceConverter.exe)
add_custom_command(
  OUTPUT ${CHM_CONVERTER}
  COMMAND ${CMD_MSBUILD} ${TOOL_SLN_FILE} 
    "/p:Platform=Any CPU"
    /p:Configuration=Release
    /p:BaseIntermediateOutputPath=${CMAKE_CURRENT_BINARY_DIR}/ChmSourceConverter/obj/
    /p:BaseOutputPath=${CMAKE_CURRENT_BINARY_DIR}/ChmSourceConverter/bin/
    /p:OutputPath=${CMAKE_CURRENT_BINARY_DIR}/ChmSourceConverter/bin/Release/
    /t:Build
    /v:q
    /nologo
  DEPENDS ${TOOL_SLN_FILE}
  COMMENT "Building ChmSourceConverter"
  VERBATIM
)

add_custom_target(build_chm_converter
  DEPENDS ${CHM_CONVERTER}
)

# Set paths
set(SRC_HELP ${CMAKE_CURRENT_SOURCE_DIR})
set(SJIS_HELP ${CMAKE_BINARY_DIR}/converted_help_source)

# Create sakura.hh from src/main/resources/sakura.hh
set(HH_INPUT ${SRC_HELP}/../src/main/resources/sakura.hh)
set(HH_OUTPUT ${SJIS_HELP}/sakura/sakura.hh)

# Copy help files to build directory
add_custom_command(
  OUTPUT ${SJIS_HELP}/macro/macro.hhp
         ${SJIS_HELP}/plugin/plugin.hhp
         ${SJIS_HELP}/sakura/sakura.hhp
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${SJIS_HELP}
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${SRC_HELP} ${SJIS_HELP}
  COMMENT "Copying help files to build directory"
  VERBATIM
)

add_custom_target(copy_help_files
  DEPENDS 
    ${SJIS_HELP}/macro/macro.hhp
    ${SJIS_HELP}/plugin/plugin.hhp
    ${SJIS_HELP}/sakura/sakura.hhp
)

# Convert UTF-8 files to SJIS
add_custom_command(
  OUTPUT
    ${HH_OUTPUT}
    ${SJIS_HELP}/.utf8_to_sjis
  COMMAND ${CMD_PWSH} -ExecutionPolicy Bypass -Command
    "Get-ChildItem -Path '${SRC_HELP}' -Recurse -Include 'CsHelp.txt','*.hhp','*.hhk','*.hhc' | ForEach-Object { Get-Content -LiteralPath $_.FullName -Encoding UTF8 | Set-Content -LiteralPath $($_.FullName -replace [regex]::Escape('\\'), '/' -replace [regex]::Escape('${SRC_HELP}'), [regex]::Escape('${SJIS_HELP}') -replace '/', [regex]::Escape('\\')) -Encoding SJIS }"
  COMMAND ${CMD_PWSH} -ExecutionPolicy Bypass -Command 
    "(Get-Content -LiteralPath '${HH_INPUT}' -Encoding UTF8) -replace '//.*' | Set-Content -LiteralPath '${HH_OUTPUT}' -Encoding SJIS"
  COMMAND ${CMAKE_COMMAND} -E touch ${SJIS_HELP}/.utf8_to_sjis
  DEPENDS
    ${HH_INPUT}
    ${SJIS_HELP}/macro/macro.hhp
    ${SJIS_HELP}/plugin/plugin.hhp
    ${SJIS_HELP}/sakura/sakura.hhp
  COMMENT "Converting UTF-8 files to SJIS"
  VERBATIM
)

add_custom_target(convert_utf8_to_sjis
  DEPENDS
    ${HH_OUTPUT}
    ${SJIS_HELP}/.utf8_to_sjis
)

add_dependencies(convert_utf8_to_sjis copy_help_files)

# Run ChmSourceConverter
add_custom_command(
  OUTPUT ${SJIS_HELP}/.converted
  COMMAND ${CHM_CONVERTER} ${SJIS_HELP}
  COMMAND ${CMAKE_COMMAND} -E touch ${SJIS_HELP}/.converted
  DEPENDS 
    ${CHM_CONVERTER}
    ${SJIS_HELP}/.utf8_to_sjis
  COMMENT "Converting CHM source files"
  VERBATIM
)

add_custom_target(convert_chm_source
  DEPENDS ${SJIS_HELP}/.converted
)

add_dependencies(convert_chm_source build_chm_converter convert_utf8_to_sjis)

# Compile CHM files
set(CHM_FILES
  ${SRC_HELP}/macro/macro.chm
  ${SRC_HELP}/plugin/plugin.chm
  ${SRC_HELP}/sakura/sakura.chm
)

foreach(CHM_TARGET macro plugin sakura)
  set(HHP_FILE ${SJIS_HELP}/${CHM_TARGET}/${CHM_TARGET}.hhp)
  set(CHM_FILE ${SRC_HELP}/${CHM_TARGET}/${CHM_TARGET}.chm)
  set(LOG_FILE ${SRC_HELP}/${CHM_TARGET}/Compile.log)
  
  add_custom_command(
    OUTPUT ${CHM_FILE}
    COMMAND ${CMAKE_COMMAND} -E env 
      "CMD_HHC=${CMD_HHC}"
      "CMD_LEPROC=${CMD_LEPROC}"
      ${CMD_PWSH} -ExecutionPolicy RemoteSigned -File 
        ${CMAKE_SOURCE_DIR}/CompileChm.ps1 
        ${HHP_FILE}
        ${CHM_FILE}
    DEPENDS 
      ${SJIS_HELP}/.converted
      ${HHP_FILE}
    COMMENT "Compiling ${CHM_TARGET}.chm"
    VERBATIM
  )
  
  add_custom_target(compile_chm_${CHM_TARGET}
    DEPENDS ${CHM_FILE}
  )
  
  add_dependencies(compile_chm_${CHM_TARGET} convert_chm_source)
endforeach()

# Main target to build all CHM files
add_custom_target(build_chm ALL
  DEPENDS 
    ${CHM_FILES}
)

add_dependencies(build_chm 
  compile_chm_macro
  compile_chm_plugin
  compile_chm_sakura
)
