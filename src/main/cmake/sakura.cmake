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

# ホストツールのプラットフォームを決める
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "AMD64|x86_64")
  set(HOST_PLATFORM "x64")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64")
  set(HOST_PLATFORM "ARM64")
else()
  set(HOST_PLATFORM "Win32")
endif()

# ホストツールのCMakeジェネレーターに渡すパラメーターを作る
if(CMAKE_GENERATOR MATCHES "^Visual Studio")
  set(GENERATOR_ARGS_FOR_HOST_TOOLS "-A ${HOST_PLATFORM} -DCMAKE_CONFIGURATION_TYPES=\"Debug\;Release\"")
else()
  set(GENERATOR_ARGS_FOR_HOST_TOOLS "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
endif()

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
  
  set(RC_FOLDER ${CMAKE_SOURCE_DIR}/${SAKURA_LANG})

  set(RESOURCE_SCRIPTS
    ${RC_FOLDER}/sakura_lang_rc.rc
    ${RC_FOLDER}/sakura_lang_rc.rc2)
  
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
    # Add include directories for sakura_lang
    target_include_directories(${SAKURA_LANG}
      PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/${RC_NAME}_${LOCALE_NAME}>"
    )

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
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<CONFIG:Release>:NDEBUG>
)

# add include directories
include_directories(
  ${CMAKE_BINARY_DIR} 
  ${CMAKE_SOURCE_DIR}/src/main/resources
  ${CMAKE_SOURCE_DIR}/sakura_core
)
