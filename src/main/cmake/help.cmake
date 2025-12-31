# CMake script for Compiled HTML Help
#
# requires
#   ${7ZIP_EXECUTABLE}
#   ${CMD_VSWHERE}
#   ${GENERATOR_ARGS_FOR_HOST_TOOLS}

set(LE_HOME "${CMAKE_SOURCE_DIR}/tools/LocaleEmulator")
set(LE_URL "https://github.com/xupefei/Locale-Emulator/releases/download/v2.5.0.1/Locale.Emulator.2.5.0.1.zip")
set(LE_ZIP "${CMAKE_SOURCE_DIR}/externals/LocaleEmulator/locale-emulator.zip")
set(CMD_LE_INSTALLER "${LE_HOME}/LEInstaller.exe")

# Find HTML Help Compiler
find_program(CMD_HHC hhc.exe
  PATHS
    "$ENV{ProgramFiles\(x86\)}/HTML Help Workshop"
  DOC "HTML Help Compiler"
)

if(NOT CMD_HHC)
  message(FATAL_ERROR "hhc.exe was not found. Please install HTML Help Workshop.")
endif()

message(STATUS "Found HTML Help Compiler: ${CMD_HHC}")

# Find PowerShell
find_program(CMD_PWSH pwsh.exe
  PATHS
    "$ENV{LOCALAPPDATA}/Microsoft/WindowsApps"
    "$ENV{ProgramFiles}/PowerShell/7"
  DOC "PowerShell Core"
)

if(NOT CMD_PWSH)
  message(FATAL_ERROR "pwsh.exe was not found.")
endif()

message(STATUS "Found PowerShell Core: ${CMD_PWSH}")

# Check system locale
cmake_language(CALL execute_process
  COMMAND ${CMD_PWSH} -NoProfile -NonInteractive -ExecutionPolicy RemoteSigned -Command
    "([System.Globalization.CultureInfo]::CurrentCulture.Name)"
  OUTPUT_VARIABLE SYSTEM_LOCALE
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "System locale: ${SYSTEM_LOCALE}")

# Find Locale Emulator (only if locale is not ja-JP)
if(NOT SYSTEM_LOCALE STREQUAL "ja-JP")
  find_program(CMD_LEPROC LEProc.exe
    PATHS "${LE_HOME}"
    DOC "Locale Emulator Processor"
  )

  if(CMD_LEPROC)
    message(STATUS "Found Locale Emulator: ${CMD_LEPROC}")
  else()
    message(STATUS "  Locale Emulator not found, download it.")
    set(LE_INSTALL "true")
    set(CMD_LEPROC "${LE_HOME}/LEProc.exe")
    find_program(CMD_AUTO_HOT_KEY AutoHotKey.exe
      PATHS
        "$ENV{ChocolateyInstall}/bin"
        "$ENV{ProgramFiles}/AutoHotKey"
      DOC "AutoHotKey"
    )
    if(NOT CMD_AUTO_HOT_KEY)
      message(FATAL_ERROR "  AutoHotKey.exe was not found.")
    endif()
    message(STATUS "  Found AutoHotKey: ${CMD_AUTO_HOT_KEY}")
  endif()
else()
  message(STATUS "  Locale Emulator not required")
endif()

# Include ChmSourceConverter.cmake for ChmSourceConverter command
include(${CMAKE_SOURCE_DIR}/src/main/cmake/ChmSourceConverter.cmake)

# Set paths
set(SRC_HELP ${CMAKE_SOURCE_DIR}/help)
set(SJIS_HELP ${CMAKE_BINARY_DIR}/converted_help_source)

# Create sakura.hh from src/main/resources/sakura.hh
set(HH_INPUT ${CMAKE_SOURCE_DIR}/src/main/resources/sakura.hh)
set(HH_OUTPUT ${SJIS_HELP}/sakura/sakura.hh)

add_custom_command(
  OUTPUT "${HH_OUTPUT}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${SJIS_HELP}/sakura"
  COMMAND ${CMD_PWSH} -NoProfile -NonInteractive -ExecutionPolicy RemoteSigned -Command 
    "(Get-Content -LiteralPath '${HH_INPUT}' -Encoding UTF8) -replace '//.*' | Set-Content -LiteralPath '${HH_OUTPUT}' -Encoding SJIS"
  DEPENDS
    "${HH_INPUT}"
  COMMENT "Converting sakura.hh to SJIS"
  VERBATIM
)

add_custom_target(copy_sakura_hh
  DEPENDS
    "${HH_OUTPUT}"
)

message(STATUS "CHM_SOURCE_CONVERTER_EXECUTABLE 1 is \"${CHM_SOURCE_CONVERTER_EXECUTABLE}\"")

add_custom_command(
  OUTPUT "${LE_ZIP}"
  COMMAND ${CMD_PWSH} -NoProfile -NonInteractive -ExecutionPolicy RemoteSigned -Command
    "Invoke-WebRequest -OutFile '${LE_ZIP}' '${LE_URL}'" 
  COMMENT "Downloading Locale Emulator"
  VERBATIM
)

add_custom_command(
  OUTPUT "${CMD_LE_INSTALLER}"
  COMMAND ${7ZIP_EXECUTABLE} x "${LE_ZIP}" "-o${LE_HOME}" "*" -y > NUL
  DEPENDS "${LE_ZIP}"
  COMMENT "Extracting Locale Emulator"
  VERBATIM
)

add_custom_command(
  OUTPUT "${CMD_LEPROC}"
  COMMAND ${CMD_AUTO_HOT_KEY} ${LE_HOME}/init-locale-emulator.ahk
  COMMAND ${CMD_LE_INSTALLER}
  DEPENDS "${CMD_LE_INSTALLER}"
  COMMENT "Installing Locale Emulator"
  VERBATIM
)

# Compile CHM files
foreach(CHM_TARGET macro plugin sakura)
  set(SRC_DIR "${CMAKE_SOURCE_DIR}/help/${CHM_TARGET}")
  set(WORK_DIR "${SJIS_HELP}/${CHM_TARGET}")
  set(HHP_FILE "${WORK_DIR}/${CHM_TARGET}.hhp")
  set(CHM_FILE "${WORK_DIR}/${CHM_TARGET}.chm")
  set(LOG_FILE "${WORK_DIR}/Compile.log")
  
  # Copy help files to build directory
  add_custom_command(
    OUTPUT "${HHP_FILE}"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${WORK_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${SRC_DIR}" "${WORK_DIR}"
    COMMENT "Copying ${CHM_TARGET} help files to build directory"
    VERBATIM
  )

  # Convert UTF-8 files to SJIS
  add_custom_command(
    OUTPUT "${WORK_DIR}/.utf8_to_sjis"
    COMMAND ${CMD_PWSH} -NoProfile -NonInteractive -ExecutionPolicy RemoteSigned -Command
      "Get-ChildItem -Path '${SRC_DIR}' -Recurse -Include 'CsHelp.txt','*.hhp','*.hhk','*.hhc' | ForEach-Object { Get-Content -LiteralPath $_.FullName -Encoding UTF8 | Set-Content -LiteralPath $($_.FullName -replace [regex]::Escape('\\'), '/' -replace [regex]::Escape('${SRC_DIR}'), [regex]::Escape('${WORK_DIR}') -replace '/', [regex]::Escape('\\')) -Encoding SJIS }"
    COMMAND ${CMAKE_COMMAND} -E touch "${WORK_DIR}/.utf8_to_sjis"
    DEPENDS "${HHP_FILE}"
    COMMENT "Converting UTF-8 files to SJIS"
    VERBATIM
  )

  # Run ChmSourceConverter
  add_custom_command(
    OUTPUT "${WORK_DIR}/.converted"
    COMMAND "${CHM_SOURCE_CONVERTER_EXECUTABLE}" "${WORK_DIR}"
    COMMAND ${CMAKE_COMMAND} -E touch "${WORK_DIR}/.converted"
    DEPENDS 
      "${CHM_SOURCE_CONVERTER_EXECUTABLE}"
      "${WORK_DIR}/.utf8_to_sjis"
      copy_sakura_hh
    COMMENT "Converting CHM source files"
    VERBATIM
  )

  # Convert slashes to  backslashes
  string(REPLACE "/" "\\" HHP_PATH "${HHP_FILE}")

  if(SYSTEM_LOCALE STREQUAL "ja-JP")
    add_custom_command(
      OUTPUT "${CHM_FILE}"
      COMMAND ${CMD_HHC} "${HHP_PATH}" || set errorlevel=0
      DEPENDS
        "${WORK_DIR}/.converted"
      COMMENT "Compiling ${CHM_TARGET}.chm"
      VERBATIM
    )
  elseif(DEFINED LE_INSTALL)
    add_custom_command(
      OUTPUT "${CHM_FILE}"
      COMMAND ${CMD_LEPROC} "${CMD_HHC}" "${HHP_PATH}" || set errorlevel=0
      COMMAND ${CMD_PWSH} -NoProfile -NonInteractive -ExecutionPolicy RemoteSigned -File
        ${CMAKE_SOURCE_DIR}/src/main/pwsh/Wait-FileAccess.ps1 "${CHM_FILE}" "${LOG_FILE}"
      DEPENDS
        "${WORK_DIR}/.converted"
        "${CMD_LEPROC}"
      COMMENT "Compiling ${CHM_TARGET}.chm with LEProc"
      VERBATIM
    )
  else()
    add_custom_command(
      OUTPUT "${CHM_FILE}"
      COMMAND ${CMD_LEPROC} "${CMD_HHC}" "${HHP_PATH}" || set errorlevel=0
      COMMAND ${CMD_PWSH} -NoProfile -NonInteractive -ExecutionPolicy RemoteSigned -File
        ${CMAKE_SOURCE_DIR}/src/main/pwsh/Wait-FileAccess.ps1 "${CHM_FILE}" "${LOG_FILE}"
      DEPENDS
        "${WORK_DIR}/.converted"
      COMMENT "Compiling ${CHM_TARGET}.chm with LEProc"
      VERBATIM
    )
  endif()

  add_custom_target(generate_${CHM_TARGET}_chm
    DEPENDS
      "${CHM_FILE}"
  )
endforeach()

set(CHM_FILES
  ${SJIS_HELP}/macro/macro.chm
  ${SJIS_HELP}/plugin/plugin.chm
  ${SJIS_HELP}/sakura/sakura.chm
)

# Main target to build all CHM files
add_custom_target(generate_all_chm ALL
  DEPENDS 
    ${CHM_FILES}
)

add_dependencies(generate_all_chm
  generate_macro_chm
  generate_plugin_chm
  generate_sakura_chm
)
