get_filename_component(REPO_ROOT "${CURRENT_PORT_DIR}/../../../.." ABSOLUTE)
set(DARKMODELIB_ROOT "${REPO_ROOT}/externals/darkmodelib")

if(NOT EXISTS "${DARKMODELIB_ROOT}/include/DarkModeSubclass.h")
  message(FATAL_ERROR "darkmodelib source not found: ${DARKMODELIB_ROOT}")
endif()

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
