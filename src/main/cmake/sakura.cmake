# sakura.cmake - sub-targets for sakura editor project

# Determine architecture macro definition
if(CMAKE_GENERATOR MATCHES "Visual Studio")
  # Visual Studio uses CMAKE_GENERATOR_PLATFORM
  if(CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
    set(RC_ARCH "_M_AMD64")
  elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64")
    set(RC_ARCH "_M_ARM64")
  elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
    set(RC_ARCH "_M_IX86")
  endif()
else()
  # MinGW uses CMAKE_SYSTEM_PROCESSOR
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64|x86_64")
    set(RC_ARCH "_M_AMD64")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64")
    set(RC_ARCH "_M_ARM64")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86|i686")
    set(RC_ARCH "_M_IX86")
  endif()
endif()

# Set DLL output directory if CMAKE_RUNTIME_OUTPUT_DIRECTORY is set
if(DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

# Create a custom command for version.h generation
add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/version.h
  COMMAND ${CMAKE_COMMAND} 
    -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
    -DBINARY_DIR=${CMAKE_BINARY_DIR}
    -P ${CMAKE_SOURCE_DIR}/src/main/cmake/version.cmake
  COMMENT "Generating version.h"
)

# Create a custom target that depends on the generated file
add_custom_target(generate_version_header ALL
  DEPENDS ${CMAKE_BINARY_DIR}/version.h
)

# Enable ExternalProject CMake's module
include(ExternalProject)

# Add External HeaderMake
ExternalProject_Add(header_make
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/tools/HeaderMake
  BINARY_DIR ${CMAKE_BINARY_DIR}/HeaderMake
  BUILD_COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/HeaderMake --config Release -- /nologo
  INSTALL_COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR}/HeaderMake --prefix ${CMAKE_BINARY_DIR}/HeaderMake
  DEPENDS generate_version_header
)

# Define the HeaderMake's path
set(HEADER_MAKE_EXECUTABLE ${CMAKE_BINARY_DIR}/HeaderMake/bin/HeaderMake.exe)

# Create a custom command for funccode_define generation
add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/Funccode_define.h
  COMMAND ${HEADER_MAKE_EXECUTABLE}
    -in=${CMAKE_SOURCE_DIR}/sakura_core/Funccode_x.hsrc
    -out=${CMAKE_BINARY_DIR}/Funccode_define.h
    -mode=define
  DEPENDS header_make
  COMMENT "Generating Funccode_define.h"
)

# Create a custom target that depends on the generated file
add_custom_target(generate_funccode_define ALL
  DEPENDS ${CMAKE_BINARY_DIR}/Funccode_define.h
)

# Create a custom command for funccode_enum generation
add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/Funccode_enum.h
  COMMAND ${HEADER_MAKE_EXECUTABLE}
    -in=${CMAKE_SOURCE_DIR}/sakura_core/Funccode_x.hsrc
    -out=${CMAKE_BINARY_DIR}/Funccode_enum.h
    -mode=enum
    -enum=EFunctionCode
  DEPENDS header_make
  COMMENT "Generating Funccode_enum.h"
)

# Create a custom target that depends on the generated file
add_custom_target(generate_funccode_enum ALL
  DEPENDS ${CMAKE_BINARY_DIR}/Funccode_enum.h
)

# Function to convert RC files from UTF-16LE to UTF-8 for MinGW
# Parameters:
#   RC_FILES_VAR - Variable name containing list of RC file paths
#   BINARY_DIR   - Directory where converted files will be placed
function(convert_rc_files_to_utf8 RC_FILES_VAR BINARY_DIR)
  # Find iconv
  find_program(ICONV_PATH iconv REQUIRED)
  
  set(RC_FILES_UTF8)
  foreach(RC_FILE ${${RC_FILES_VAR}})
    get_filename_component(RC_NAME ${RC_FILE} NAME_WE)
    get_filename_component(RC_EXT ${RC_FILE} EXT)
    set(UTF8_RC_FILE ${BINARY_DIR}/${RC_NAME}${RC_EXT})
    
    add_custom_command(
      OUTPUT ${UTF8_RC_FILE}
      COMMAND ${ICONV_PATH} -f UTF-16LE -t UTF-8 "${RC_FILE}" > "${UTF8_RC_FILE}"
      DEPENDS ${RC_FILE}
      COMMENT "Converting ${RC_NAME}${RC_EXT} from UTF-16LE to UTF-8 using iconv"
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
  set(PROJECT_NAME sakura_lang_${LOCALE_NAME_UNDERSCORE})
  project(${PROJECT_NAME} LANGUAGES CXX)
  
  set(RC_FOLDER ${CMAKE_SOURCE_DIR}/${PROJECT_NAME})

  set(RESOURCE_SCRIPTS
    ${RC_FOLDER}/sakura_lang_rc.rc
    ${RC_FOLDER}/sakura_lang_rc.rc2)
  
  if(MINGW)
    # Convert RC files to UTF-8 for MinGW
    convert_rc_files_to_utf8(RESOURCE_SCRIPTS ${CMAKE_CURRENT_BINARY_DIR})
  endif(MINGW)
  
  # Create the library
  add_library(${PROJECT_NAME} MODULE ${RESOURCE_SCRIPTS})
  
  # Set include directories
  target_include_directories(${PROJECT_NAME}
    PRIVATE 
      ${CMAKE_CURRENT_BINARY_DIR} 
      ${CMAKE_SOURCE_DIR}/src/main/resources
      ${CMAKE_SOURCE_DIR}/sakura_core
  )
  
  # Add dependencies
  add_dependencies(${PROJECT_NAME} generate_version_header generate_funccode_define)
  
  # Set target properties
  set_target_properties(${PROJECT_NAME} PROPERTIES
    LINKER_LANGUAGE "CXX"
  )

  # MSVC specific settings
  if(MSVC)
    # Convert decimal LOCALE_ID to hexadecimal for MSVC RC
    math(EXPR LOCALE_ID_HEX "${LOCALE_ID}" OUTPUT_FORMAT HEXADECIMAL)

    # Set RC flags for MSVC
    target_compile_options(${PROJECT_NAME} PRIVATE
      $<$<COMPILE_LANGUAGE:RC>:/c utf-8 /l ${LOCALE_ID_HEX} /D${RC_ARCH}>
    )

    # avoid error LNK2001 for "__DllMainCRTStartup@12"
    set_target_properties(${PROJECT_NAME} PROPERTIES
      LINK_FLAGS "/NOENTRY"
    )
  endif(MSVC)
  
  # MinGW specific settings
  if(MINGW)
    # Set RC flags for MinGW (windres uses decimal)
    target_compile_options(${PROJECT_NAME} PRIVATE
      $<$<COMPILE_LANGUAGE:RC>:-c 65001 -l ${LOCALE_ID} -D${RC_ARCH}>
    )

    # avoid prefixing of DLL name, set PREFIX to blank.
    # https://cmake.org/cmake/help/v3.12/prop_tgt/PREFIX.html?highlight=prefix
    set_target_properties(${PROJECT_NAME} PROPERTIES
      PREFIX ""
    )
  endif(MINGW)
endfunction()
