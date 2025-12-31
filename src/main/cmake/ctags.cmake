# CMake script for ctags
#
# requires
#   ${7ZIP_EXECUTABLE}
#   ${ARCH}
#   ${CMAKE_GENERATOR_PLATFORM}
#   ${CMAKE_GENERATOR}
#   ${CMD_VS_DEV}
#   ${GIT_EXECUTABLE}
#   ${HOST_ARCH}
#   ${OUTPUT_DIRECTORY}

# Define the ctags's path
set(CTAGS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/externals/ctags")
set(CTAGS_BUILD_DIR "${CMAKE_BINARY_DIR}/ctags")
set(CTAGS_VERSION "v6.1.0")
set(CTAGS_ZIP_FILE "${CMAKE_SOURCE_DIR}/installer/externals/universal-ctags/ctags-${CTAGS_VERSION}-${ARCH}.zip")
set(CTAGS_GENERATED "${CTAGS_BUILD_DIR}/ctags.exe")
set(CTAGS_EXECUTABLE "${OUTPUT_DIRECTORY}/ctags.exe")

# 実績のあるものだけビルド対象にする。
if(CMAKE_GENERATOR MATCHES "^Visual Studio" AND NOT CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
  add_custom_command(
    OUTPUT
      "${CMAKE_SOURCE_DIR}/externals/ctags/.git"
    COMMAND ${CMAKE_COMMAND}
      -DGIT_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
      -DREPO_ROOT:PATH=${CMAKE_SOURCE_DIR}
      -DSUBMODULE_PATH:STRING=externals/ctags
      -DLOCK_PATH:FILEPATH=${CMAKE_BINARY_DIR}/cmake-submodule-update.lock
      -P ${CMAKE_SOURCE_DIR}/src/main/cmake/git_submodule_update_locked.cmake
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Fetching ctag's source files"
  )

  add_custom_target(fetch_ctags_source_files
    DEPENDS
      "${CMAKE_SOURCE_DIR}/externals/ctags/.git"
  )

  # Copy source files to build directory
  add_custom_command(
    OUTPUT "${CTAGS_BUILD_DIR}/mk_mvc.mak"
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CTAGS_BUILD_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CTAGS_SOURCE_DIR} ${CTAGS_BUILD_DIR}
    DEPENDS fetch_ctags_source_files
    COMMENT "Copying ctags's source files to build directory"
    VERBATIM
  )

  add_custom_target(copy_ctags_source_files
    DEPENDS
      "${CTAGS_BUILD_DIR}/mk_mvc.mak"
  )

  add_custom_command(
    OUTPUT "${CTAGS_GENERATED}"
    COMMAND call "${CMD_VS_DEV}" -host_arch=${HOST_ARCH} -arch=${HOST_ARCH} && nmake -f mk_mvc.mak
    WORKING_DIRECTORY "${CTAGS_BUILD_DIR}"
    DEPENDS copy_ctags_source_files
    COMMENT "Building ctags"
  )

  add_custom_command(
    OUTPUT "${CTAGS_EXECUTABLE}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CTAGS_GENERATED}" "${CTAGS_EXECUTABLE}"
    DEPENDS "${CTAGS_GENERATED}"
    COMMENT "Copying ctags.exe to output directory"
  )
else()
  add_custom_command(
    OUTPUT "${CTAGS_EXECUTABLE}"
    COMMAND ${7ZIP_EXECUTABLE} e "${CTAGS_ZIP_FILE}" -o"${OUTPUT_DIRECTORY}" -y ctags.exe > NUL
    COMMENT "Extracting ctags.exe from ${CTAGS_ZIP_FILE}"
  )
endif()

add_custom_target(generate_ctags ALL
  DEPENDS
    "${CTAGS_EXECUTABLE}"
)
