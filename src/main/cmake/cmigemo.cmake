# CMake script for cmigemo
#
# requires
#   ${ARCH}
#   ${CMD_PWSH}
#   ${CMD_VS_DEV}
#   ${GIT_EXECUTABLE}
#   ${HOST_ARCH}
#   ${ICONV_PATH}
#   ${OUTPUT_DIRECTORY}

# Define the cmigemo's path
set(CMIGEMO_SOURCE_DIR "${CMAKE_SOURCE_DIR}/externals/cmigemo")
set(CMIGEMO_DICT_SOURCE_DIR "${CMAKE_SOURCE_DIR}/externals/cmigemo-dict")
set(CMIGEMO_BUILD_DIR "${CMAKE_BINARY_DIR}/cmigemo")
set(CMIGEMO_GENERATED "${CMIGEMO_BUILD_DIR}/build/migemo.dll")
set(CMIGEMO_DLL "${OUTPUT_DIRECTORY}/migemo.dll")
set(CMIGEMO_INCLUDE_DIR "${CMAKE_BINARY_DIR}/include/cmigemo")

add_custom_command(
  OUTPUT
    "${CMIGEMO_SOURCE_DIR}/.git"
  COMMAND ${CMAKE_COMMAND}
    -DGIT_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
    -DREPO_ROOT:PATH=${CMAKE_SOURCE_DIR}
    -DSUBMODULE_PATH:STRING=externals/cmigemo
    -DLOCK_PATH:FILEPATH=${CMAKE_BINARY_DIR}/cmake-submodule-update.lock
    -P ${CMAKE_SOURCE_DIR}/src/main/cmake/git_submodule_update_locked.cmake
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  COMMENT "Fetching cmigemo's source files"
)

add_custom_target(fetch_cmigemo_source_files
  DEPENDS
    "${CMIGEMO_SOURCE_DIR}/.git"
)

# Copy source files to build directory
add_custom_command(
  OUTPUT "${CMIGEMO_BUILD_DIR}/Makefile"
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMIGEMO_BUILD_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMIGEMO_SOURCE_DIR} ${CMIGEMO_BUILD_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMIGEMO_DICT_SOURCE_DIR}/cmigemo-dict-utf-8.zip" "${CMAKE_BINARY_DIR}/cmigemo-dict-utf-8.zip"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMIGEMO_DICT_SOURCE_DIR}/cmigemo-dict-cp932.zip" "${CMAKE_BINARY_DIR}/cmigemo-dict-cp932.zip"
  DEPENDS fetch_cmigemo_source_files
  COMMENT "Copying cmigemo's source files to build directory"
  VERBATIM
)

add_custom_target(copy_cmigemo_source_files
  DEPENDS
    "${CMIGEMO_BUILD_DIR}/Makefile"
)

add_custom_command(
  OUTPUT "${CMIGEMO_GENERATED}"
  COMMAND call "${CMD_VS_DEV}" -host_arch=${HOST_ARCH} -arch=${ARCH} && nmake -f compile/Make_mvc.mak DEFINES=-D_CRT_SECURE_NO_WARNINGS
  WORKING_DIRECTORY "${CMIGEMO_BUILD_DIR}"
  DEPENDS copy_cmigemo_source_files
  COMMENT "Building migemo.dll"
)

add_custom_command(
  OUTPUT "${CMIGEMO_DLL}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMIGEMO_GENERATED}" "${CMIGEMO_DLL}"
  DEPENDS "${CMIGEMO_GENERATED}"
  COMMENT "Copying migemo.dll to output directory"
)

if(MSVC)
  add_custom_command(
    OUTPUT "${CMIGEMO_INCLUDE_DIR}/migemo.h"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMIGEMO_INCLUDE_DIR}"
    COMMAND ${CMD_PWSH} -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/src/main/pwsh/sjis_to_utf8bom.ps1" -File "${CMIGEMO_BUILD_DIR}/src/migemo.h" -Destination "${CMIGEMO_INCLUDE_DIR}/migemo.h"
    DEPENDS copy_cmigemo_source_files
    COMMENT "Copying cmigemo/migemo.h to include directory"
  )
endif()

if(MINGW)
  add_custom_command(
    OUTPUT "${CMIGEMO_INCLUDE_DIR}/migemo.h"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMIGEMO_INCLUDE_DIR}"
    COMMAND ${ICONV_PATH} -f cp932 -t UTF-8 "${CMIGEMO_BUILD_DIR}/src/migemo.h" > "${CMIGEMO_INCLUDE_DIR}/migemo.h.nobom"
    COMMAND bash -lc \"printf '\\xEF\\xBB\\xBF' > ${CMIGEMO_INCLUDE_DIR}/migemo.h && cat ${CMIGEMO_INCLUDE_DIR}/migemo.h.nobom >> ${CMIGEMO_INCLUDE_DIR}/migemo.h\"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMIGEMO_INCLUDE_DIR}/migemo.h.nobom"
    DEPENDS copy_cmigemo_source_files
    COMMENT "Copying cmigemo/migemo.h to include directory"
  )
endif()

add_custom_target(generate_cmigemo ALL
  DEPENDS
    "${CMIGEMO_DLL}"
    "${CMIGEMO_INCLUDE_DIR}/migemo.h"
)
