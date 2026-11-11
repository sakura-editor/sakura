get_filename_component(REPO_ROOT "${CURRENT_PORT_DIR}/../../../.." ABSOLUTE)
set(DARKMODELIB_SOURCE_ROOT "${REPO_ROOT}/externals/darkmodelib")

if(NOT EXISTS "${DARKMODELIB_SOURCE_ROOT}/include/Darkmodelib.h")
  message(FATAL_ERROR "darkmodelib source not found: ${DARKMODELIB_SOURCE_ROOT}")
endif()

# Do not patch the submodule checkout; apply the fix to a build-tree copy.
set(DARKMODELIB_ARCHIVE "${CURRENT_BUILDTREES_DIR}/darkmodelib-source.tar")
vcpkg_execute_required_process(
  COMMAND "${CMAKE_COMMAND}" -E tar cf "${DARKMODELIB_ARCHIVE}" --format=gnutar
    src include LICENSE.md LICENSE-MIT.md
  WORKING_DIRECTORY "${DARKMODELIB_SOURCE_ROOT}"
  LOGNAME archive-source
)

vcpkg_extract_source_archive(
  DARKMODELIB_ROOT
  ARCHIVE "${DARKMODELIB_ARCHIVE}"
  NO_REMOVE_ONE_LEVEL
  PATCHES selected-tab-clip.patch
)

vcpkg_cmake_configure(
  SOURCE_PATH "${CURRENT_PORT_DIR}"
  OPTIONS
    "-DDARKMODELIB_ROOT=${DARKMODELIB_ROOT}"
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH share/darkmodelib)

configure_file("${CMAKE_CURRENT_LIST_DIR}/usage" "${CURRENT_PACKAGES_DIR}/share/${PORT}/usage" COPYONLY)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

if(EXISTS "${DARKMODELIB_ROOT}/LICENSE.md")
  file(INSTALL
    "${DARKMODELIB_ROOT}/LICENSE.md"
    DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
    RENAME copyright
  )
elseif(EXISTS "${DARKMODELIB_ROOT}/LICENSE-MIT.md")
  file(INSTALL
    "${DARKMODELIB_ROOT}/LICENSE-MIT.md"
    DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
    RENAME copyright
  )
endif()
