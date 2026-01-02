# sakura.cmake - sub-targets for sakura editor project
#

# exeの出力先フォルダーを決める
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

# ビルド対象のCPUアーキテクチャを決める
if(CMAKE_GENERATOR MATCHES "^Visual Studio")
  # VSジェネレーターは -A の値で判定する
  if(CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
    set(ARCH "x64")
  elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64")
    set(ARCH "arm64")
  elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
    set(ARCH "x86")
  endif()

  # CMakeジェネレーターに渡すパラメーターを作る
  set(GENERATOR_ARGS "-A ${CMAKE_GENERATOR_PLATFORM} -DCMAKE_CONFIGURATION_TYPES=\"Debug\;Release\"")

else()
  # CMakeが持ってる値を整形する
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64|x86_64")
    set(ARCH "x64")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64")
    set(ARCH "arm64")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86|i686")
    set(ARCH "x86")
  endif()

  # CMakeジェネレーターに渡すパラメータを作る
  set(GENERATOR_ARGS "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
endif()

# ホストツールのプラットフォームとCPUを決める
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "AMD64|x86_64")
  set(HOST_PLATFORM "x64")
  set(HOST_ARCH "x64")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64")
  set(HOST_PLATFORM "ARM64")
  set(HOST_ARCH "arm64")
else()
  set(HOST_PLATFORM "Win32")
  set(HOST_ARCH "x86")
endif()

# ホストツールのCMakeジェネレーターに渡すパラメーターを作る
if(CMAKE_GENERATOR MATCHES "^Visual Studio")
  set(GENERATOR_ARGS_FOR_HOST_TOOLS "-A ${HOST_PLATFORM} -DCMAKE_CONFIGURATION_TYPES=\"Debug\;Release\"")
else()
  set(GENERATOR_ARGS_FOR_HOST_TOOLS "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
endif()

# vswhereを探す。（なくてもOK）
find_program(CMD_VSWHERE vswhere.exe
  PATHS
    "$ENV{ChocolateyInstall}"
    "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/Installer"
  PATH_SUFFIXES
    "bin"
  DOC "Visual Studio Locator"
)

if(CMD_VSWHERE)
  message(STATUS "Found vswhere: ${CMD_VSWHERE}")
endif()

# 環境変数とvswhereを使ってVSバージョンを取得する
if($ENV{VisualStudioVersion})
  set(VISUAL_STUDIO_VERSION "$ENV{VisualStudioVersion}")
elseif(CMD_VSWHERE)
  # Use vswhere to get Visual Studio version
  execute_process(
    COMMAND ${CMD_VSWHERE} -latest -property installationVersion
    OUTPUT_VARIABLE VISUAL_STUDIO_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()

# VSバージョンが取れた場合VsDevCmdを探す
if(VISUAL_STUDIO_VERSION)
  # extract major version
  string(REGEX REPLACE "([0-9]+)\\..+" "\\1" VS_VERSION "${VISUAL_STUDIO_VERSION}")

  # Use vswhere to find VsDevCmd.bat
  execute_process(
    COMMAND ${CMD_VSWHERE} -find "Common7\\Tools\\VsDevCmd.bat" -version "${VS_VERSION}"
    OUTPUT_VARIABLE CMD_VS_DEV
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  # Convert backslashes to forward slashes
  string(REPLACE "\\" "/" CMD_VS_DEV "${CMD_VS_DEV}")

  if(CMD_VS_DEV)
    message(STATUS "Found VsDevCmd: ${CMD_VS_DEV}")
  endif()
endif(VISUAL_STUDIO_VERSION)

# Find Git with additional search paths
find_program(GIT_EXECUTABLE git
  PATHS
    "$ENV{ProgramFiles}/Git"
  PATH_SUFFIXES
    cmd
    bin
)

if(NOT GIT_EXECUTABLE)
  message(FATAL_ERROR "Git not found")
endif()

message(STATUS "Found Git: ${GIT_EXECUTABLE}")

# Find patch.exe from Git installation
find_program(PATCH_EXECUTABLE patch
  PATHS
    "$ENV{ProgramFiles}/Git/usr/bin"
    "$ENV{LOCALAPPDATA}/Programs/Git/usr/bin"
  NO_DEFAULT_PATH
  DOC "patch.exe command from Git"
)

if(NOT PATCH_EXECUTABLE)
  message(FATAL_ERROR "patch.exe was not found. Please install Git.")
endif()

message(STATUS "Found patch.exe: ${PATCH_EXECUTABLE}")

# Find PowerShell Core(required)
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

# Find 7zip for archive extraction
find_program(7ZIP_EXECUTABLE 7z
  PATHS
    "$ENV{ChocolateyInstall}"
)

if(NOT 7ZIP_EXECUTABLE)
  message(FATAL_ERROR "7z.exe not found")
endif()

message(STATUS "Found 7z: ${7ZIP_EXECUTABLE}")

# Create a custom command for banner generation
add_custom_target(show_dev_banner ALL
  COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan "-------------------------------------------------------------------------------------"
  COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan "---  This is a Dev version and under development. Be careful to use this version. ---"
  COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan "-------------------------------------------------------------------------------------"
  VERBATIM
)

# Include compiletests.cmake
include(${CMAKE_SOURCE_DIR}/src/test/cmake/compiletests.cmake)

# Create a custom command for version.h generation
add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/version.h"
  COMMAND ${CMAKE_COMMAND} 
    -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
    -DGIT_EXECUTABLE=${GIT_EXECUTABLE}
    -P ${CMAKE_SOURCE_DIR}/src/main/cmake/version.cmake
  COMMENT "Generating version.h"
)

# Create a custom target that depends on the generated file
add_custom_target(generate_version_header
  DEPENDS
    "${CMAKE_BINARY_DIR}/version.h"
)

# Include HeaderMake.cmake for HeaderMake command
include(${CMAKE_SOURCE_DIR}/src/main/cmake/HeaderMake.cmake)

# Create a custom command for funccode_define generation
add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/Funccode_define.h"
  COMMAND ${HEADER_MAKE_EXECUTABLE}
    -in=${CMAKE_SOURCE_DIR}/sakura_core/Funccode_x.hsrc
    -out=${CMAKE_BINARY_DIR}/Funccode_define.h
    -mode=define
  DEPENDS generate_header_make
  COMMENT "Generating Funccode_define.h"
)

# Create a custom target that depends on the generated file
add_custom_target(generate_funccode_define
  DEPENDS
    "${CMAKE_BINARY_DIR}/Funccode_define.h"
)

# Create a custom command for funccode_enum generation
add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/Funccode_enum.h"
  COMMAND ${HEADER_MAKE_EXECUTABLE}
    -in=${CMAKE_SOURCE_DIR}/sakura_core/Funccode_x.hsrc
    -out=${CMAKE_BINARY_DIR}/Funccode_enum.h
    -mode=enum
    -enum=EFunctionCode
  DEPENDS generate_header_make
  COMMENT "Generating Funccode_enum.h"
)

# Create a custom target that depends on the generated file
add_custom_target(generate_funccode_enum
  DEPENDS
    "${CMAKE_BINARY_DIR}/Funccode_enum.h"
)

if(MINGW)
  # Find iconv
  find_program(ICONV_PATH iconv REQUIRED)

  if(NOT ICONV_PATH)
    message(FATAL_ERROR "iconv was not found.")
  endif()

  message(STATUS "Found iconv: ${ICONV_PATH}")
endif(MINGW)

# Function to convert RC files from UTF-16LE to UTF-8 for MinGW
# Parameters:
#   RC_FILES_VAR - Variable name containing list of RC file paths
#   LOCALE_NAME  - Locale name
#   BINARY_DIR   - Directory where converted files will be placed
function(convert_rc_files_to_utf8 RC_FILES_VAR LOCALE_NAME BINARY_DIR)
  set(RC_FILES_UTF8)
  foreach(RC_FILE ${${RC_FILES_VAR}})
    get_filename_component(RC_NAME ${RC_FILE} NAME_WE)
    get_filename_component(RC_EXT ${RC_FILE} EXT)
    set(UTF8_RC_FILE ${BINARY_DIR}/${RC_NAME}_${LOCALE_NAME}/${RC_NAME}${RC_EXT})
    
    add_custom_command(
      OUTPUT ${UTF8_RC_FILE}
      COMMAND ${ICONV_PATH} -f UTF-16LE -t UTF-8 "${RC_FILE}" > "${UTF8_RC_FILE}"
      DEPENDS ${RC_FILE}
      COMMENT "Converting ${RC_NAME}_${LOCALE_NAME}${RC_EXT} from UTF-16LE to UTF-8 using iconv"
    )
    
    list(APPEND RC_FILES_UTF8 ${UTF8_RC_FILE})
  endforeach()
  
  # Replace the original variable with UTF-8 converted files
  set(${RC_FILES_VAR} ${RC_FILES_UTF8} PARENT_SCOPE)
endfunction()

# Function to create a language DLL project
# Parameters:
#   LOCALE_NAME  - Name of the locale (e.g., en-US, zh-CN)
#   LOCALE_ID    - Locale identifier in decimal (e.g., 1033 for en-US, 2052 for zh-CN)
function(create_language_dll LOCALE_NAME LOCALE_ID)
  string(REPLACE "-" "_" LOCALE_NAME_UNDERSCORE "${LOCALE_NAME}")
  set(SAKURA_LANG sakura_lang_${LOCALE_NAME_UNDERSCORE})
  
  set(RC_FOLDER ${CMAKE_SOURCE_DIR}/sakura_lang)

  set(RESOURCE_SCRIPTS
    ${RC_FOLDER}/sakura_rc_${LOCALE_NAME}.rc
    ${RC_FOLDER}/sakura_rc_${LOCALE_NAME}.rc2)
  
  if(MINGW)
    # Convert RC files to UTF-8 for MinGW
    convert_rc_files_to_utf8(RESOURCE_SCRIPTS "${LOCALE_NAME}" ${CMAKE_CURRENT_BINARY_DIR})
  endif(MINGW)
  
  # Create the library
  add_library(${SAKURA_LANG} MODULE ${RESOURCE_SCRIPTS})

  # Add dependencies
  add_dependencies(${SAKURA_LANG}
    generate_version_header
    generate_funccode_define
  )
  
  # Set target properties
  set_target_properties(${SAKURA_LANG}
    PROPERTIES
      # ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
      LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
      LINKER_LANGUAGE "CXX"
      PDB_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
      # RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
  )

  # MSVC specific settings
  if(MSVC)
    # Convert decimal LOCALE_ID to hexadecimal for MSVC RC
    math(EXPR LOCALE_ID_HEX "${LOCALE_ID}" OUTPUT_FORMAT HEXADECIMAL)

    # Set RC flags for MSVC
    target_compile_options(${SAKURA_LANG}
      PRIVATE
        /l ${LOCALE_ID_HEX}
    )

    # avoid error LNK2001 for "__DllMainCRTStartup@12"
    set_target_properties(${SAKURA_LANG}
      PROPERTIES
        LINK_FLAGS "/NOENTRY"
    )
  endif(MSVC)
  
  # MinGW specific settings
  if(MINGW)
    # Set RC flags for MinGW (windres uses decimal)
    target_compile_options(${SAKURA_LANG}
      PRIVATE
        "$<$<COMPILE_LANGUAGE:RC>:-c 65001-l ${LOCALE_ID} --use-temp-file>"
    )

    # avoid prefixing of DLL name, set PREFIX to blank.
    # https://cmake.org/cmake/help/v3.12/prop_tgt/PREFIX.html?highlight=prefix
    set_target_properties(${SAKURA_LANG}
      PROPERTIES
        PREFIX ""
    )
  endif(MINGW)
endfunction(create_language_dll)

# add global definitions
add_compile_definitions(
  UNICODE
  _UNICODE
  _WIN32_WINNT=_WIN32_WINNT_WIN10
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<CONFIG:Release>:NDEBUG>
)

# add include directories
include_directories(
  ${CMAKE_BINARY_DIR} 
  ${CMAKE_SOURCE_DIR}/src/main/cpp
  ${CMAKE_SOURCE_DIR}/src/main/resources
  ${CMAKE_SOURCE_DIR}/sakura_core
)

if(MSVC)
  # VCランタイムを指定する（/MTd, /MTにする）
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

  # 静的リンクするライブラリの構築に使うパラメーターを作る
  set(GENERATOR_ARGS_FOR_STATIC_LIBRARY "\"-DCMAKE_MSVC_RUNTIME_LIBRARY=${CMAKE_MSVC_RUNTIME_LIBRARY}\"")

  add_compile_options(
    /source-charset:utf-8
    /execution-charset:shift_jis
    /wd4996
  )
endif(MSVC)

if(MINGW)
  add_compile_options(
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Release>:-O2>
    -MMD
    -finput-charset=utf-8
    -fexec-charset=cp932
    -Wdeprecated-declarations
  )
endif(MINGW)

# define header files of sakura-editor
file(GLOB_RECURSE HEADERS
  ${CMAKE_SOURCE_DIR}/src/main/cpp/*.hpp
  ${CMAKE_SOURCE_DIR}/src/main/cpp/*.h
  ${CMAKE_SOURCE_DIR}/sakura_core/*.hpp
  ${CMAKE_SOURCE_DIR}/sakura_core/*.h
)

# define source files of sakura_core
file(GLOB_RECURSE SOURCES
  ${CMAKE_SOURCE_DIR}/src/main/cpp/*.cpp
  ${CMAKE_SOURCE_DIR}/sakura_core/*.cpp
)

set(RESOURCE_SCRIPTS
  ${CMAKE_SOURCE_DIR}/sakura_core/sakura_rc.rc
  ${CMAKE_SOURCE_DIR}/sakura_core/sakura_rc.rc2
)

set(NATVIS_FILES
  ${CMAKE_SOURCE_DIR}/src/main/resources/sakura.natvis
)

# define precompiled headers
set(PCH_HEADER ${CMAKE_SOURCE_DIR}/sakura_core/StdAfx.h)

if(MINGW)
  # Convert RC files to UTF-8 for MinGW
  convert_rc_files_to_utf8(RESOURCE_SCRIPTS "ja-JP" ${CMAKE_BINARY_DIR})
endif(MINGW)

# Create sakura_core object library
add_library(sakura_core OBJECT ${PCH_HEADER} ${SOURCES} ${RESOURCE_SCRIPTS} ${HEADERS})

# Enable precompiled headers for sakura_core
target_precompile_headers(sakura_core PRIVATE ${PCH_HEADER})

# Set C++ standard for sakura_core
target_compile_features(sakura_core PUBLIC cxx_std_20)

# Add include directories for sakura_core
target_include_directories(sakura_core
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>"
)

# Add link directories for sakura_core
target_link_directories(sakura_core
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/lib$<$<CONFIG:Debug>:/Debug>>"
)

# link libraries
target_link_libraries(sakura_core
  PUBLIC
    comctl32
    dwmapi
    htmlhelp
    imm32
    mpr
    msimg32
    ole32
    oleaut32
    shlwapi
    uuid
    uxtheme
    windowscodecs
    winmm
    winspool
)

# Add dependencies for sakura_core
add_dependencies(sakura_core
  generate_version_header
  generate_funccode_define
  generate_funccode_enum
  generate_cmigemo
)

if(MSVC)
  target_sources(sakura_core
    PUBLIC
      ${NATVIS_FILES}
  )
  set_target_properties(sakura_core
    PROPERTIES
      VS_DEBUGGER_VISUALIZER "${NATVIS_FILES}"
  )
  # add definitions for sakura_core
  target_compile_definitions(sakura_core
    PUBLIC
      NOMINMAX
  )
  # add compile options for sakura_core
  target_compile_options(sakura_core
    PRIVATE
      /FAsu
      /Fa"${CMAKE_BINARY_DIR}"
  )
endif(MSVC)

if(MINGW)
  # Set RC flags for MinGW (windres uses decimal)
  set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} -c 65001 -l 1041 --use-temp-file")

  # Add include directories for sakura_core
  target_include_directories(sakura_core
    PRIVATE
      "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/sakura_rc_ja-JP>"
  )

  target_link_options(sakura_core
    PUBLIC
      -municode
      -static
      $<$<CONFIG:Release>:-s>
  )
endif(MINGW)
