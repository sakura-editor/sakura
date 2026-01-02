# CMake script for bregonig
#
# requires
#   ${7ZIP_EXECUTABLE}
#   ${ARCH}
#   ${CMAKE_GENERATOR}
#   ${CMD_VS_DEV}
#   ${GIT_EXECUTABLE}
#   ${HOST_ARCH}
#   ${OUTPUT_DIRECTORY}
#   ${PATCH_EXECUTABLE}

# Define the bregonig's path
set(BREGONIG_SOURCE_DIR "${CMAKE_SOURCE_DIR}/externals/bregonig")
set(BREGONIG_BUILD_DIR "${CMAKE_BINARY_DIR}/bregonig")
set(BREGONIG_ZIP_FILE "${CMAKE_SOURCE_DIR}/installer/externals/bregonig/bron420.zip")
set(BREGONIG_GENERATED "${BREGONIG_BUILD_DIR}/src/obj${ARCH}/bregonig.dll")
set(BREGONIG_DLL "${OUTPUT_DIRECTORY}/bregonig.dll")

if(CMAKE_GENERATOR MATCHES "^Visual Studio")
  add_custom_command(
    OUTPUT
      "${CMAKE_SOURCE_DIR}/externals/bregonig/.git"
    COMMAND ${CMAKE_COMMAND}
      -DGIT_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
      -DREPO_ROOT:PATH=${CMAKE_SOURCE_DIR}
      -DSUBMODULE_PATH:STRING=externals/bregonig
      -DLOCK_PATH:FILEPATH=${CMAKE_BINARY_DIR}/cmake-submodule-update.lock
      -P ${CMAKE_SOURCE_DIR}/src/main/cmake/git_submodule_update_locked.cmake
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Fetching bregonig's source files"
  )

  add_custom_target(fetch_bregonig_source_files
    DEPENDS
      "${CMAKE_SOURCE_DIR}/externals/bregonig/.git"
  )

  # Copy source files to build directory
  add_custom_command(
    OUTPUT "${BREGONIG_BUILD_DIR}/src/Makefile"
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${BREGONIG_BUILD_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${BREGONIG_SOURCE_DIR} ${BREGONIG_BUILD_DIR}
    COMMAND ${PATCH_EXECUTABLE} -d ${BREGONIG_BUILD_DIR} -p1 < ${CMAKE_SOURCE_DIR}/externals/bregonig_0001-DYNAMICBASE.patch
    DEPENDS fetch_bregonig_source_files
    COMMENT "Copying bregonig's source files to build directory and applying patches"
    VERBATIM
  )

  add_custom_target(copy_bregonig_source_files
    DEPENDS
      "${BREGONIG_BUILD_DIR}/src/Makefile"
  )

  include(${CMAKE_SOURCE_DIR}/src/main/cmake/Onigmo.cmake)

  add_custom_command(
    OUTPUT "${BREGONIG_GENERATED}"
    COMMAND call "${CMD_VS_DEV}" -host_arch=${HOST_ARCH} -arch=${ARCH} && nmake ONIG_DIR=..\\..\\Onigmo TARGET_CPU=${ARCH}
    WORKING_DIRECTORY "${BREGONIG_BUILD_DIR}/src"
    DEPENDS
      copy_bregonig_source_files
      generate_onigmo_library
    COMMENT "Building bregonig.dll"
  )

  add_custom_command(
    OUTPUT "${BREGONIG_DLL}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${BREGONIG_GENERATED}" "${BREGONIG_DLL}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/include/bregonig"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${BREGONIG_BUILD_DIR}/src/bregexp.h" "${CMAKE_BINARY_DIR}/include/bregonig/bregexp.h"
    DEPENDS "${BREGONIG_GENERATED}"
    COMMENT "Copying bregonig.dll to output directory"
  )
else()
  if(ARCH STREQUAL "x64")
    set(BREGONIG_EXTRACT_TARGET "x64/bregonig.dll")
  else()
    set(BREGONIG_EXTRACT_TARGET "bregonig.dll")
  endif()

  add_custom_command(
    OUTPUT "${BREGONIG_DLL}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIRECTORY}"
    COMMAND ${7ZIP_EXECUTABLE} e "${BREGONIG_ZIP_FILE}" -o"${OUTPUT_DIRECTORY}" -y "${BREGONIG_EXTRACT_TARGET}" > NUL
    COMMENT "Extracting bregonig.dll from ${BREGONIG_ZIP_FILE}"
  )
endif()

add_custom_target(generate_bregonig ALL
  DEPENDS
    "${BREGONIG_DLL}"
)

