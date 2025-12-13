# CMake script for Onigmo
#
# requires
#   ${CMD_VS_DEV}
#   ${ARCH}
#   ${HOST_ARCH}

# Define the Onigmo's path
set(ONIGMO_SOURCE_DIR "${CMAKE_SOURCE_DIR}/externals/Onigmo")
set(ONIGMO_BUILD_DIR "${CMAKE_BINARY_DIR}/Onigmo")
set(ONIGMO_LIB "${ONIGMO_BUILD_DIR}/build_${ARCH}/onigmo.lib")

if(CMD_VS_DEV)
  add_custom_command(
    OUTPUT
      "${CMAKE_SOURCE_DIR}/externals/Onigmo/.git"
    COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive --depth 1 --recommend-shallow externals/Onigmo
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Fetching Onigmo's source files"
  )

  add_custom_target(fetch_onigmo_source_files
    DEPENDS
      "${CMAKE_SOURCE_DIR}/externals/Onigmo/.git"
  )

  # Copy source files to build directory
  add_custom_command(
    OUTPUT "${ONIGMO_BUILD_DIR}/win32/Makefile"
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${ONIGMO_BUILD_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${ONIGMO_SOURCE_DIR} ${ONIGMO_BUILD_DIR}
    DEPENDS fetch_onigmo_source_files
    COMMENT "Copying Onigmo's source files to build directory"
    VERBATIM
  )

  add_custom_target(copy_onigmo_source_files
    DEPENDS
      "${ONIGMO_BUILD_DIR}/win32/Makefile"
  )

  add_custom_command(
    OUTPUT "${ONIGMO_LIB}"
    COMMAND call "${CMD_VS_DEV}" -host_arch=${HOST_ARCH} -arch=${ARCH} && build_nmake.cmd ARCH=${ARCH}
    WORKING_DIRECTORY "${ONIGMO_BUILD_DIR}"
    DEPENDS copy_onigmo_source_files
    COMMENT "Building Onigmo Library"
  )

  add_custom_target(generate_onigmo_library
    DEPENDS
      "${ONIGMO_LIB}"
  )
endif(CMD_VS_DEV)
