get_filename_component(REPO_ROOT "${CURRENT_PORT_DIR}/../../../.." ABSOLUTE)
set(DLL_PLUGIN1_SOURCE_DIR "${REPO_ROOT}/tools/dll_plugin1")

if(NOT EXISTS "${DLL_PLUGIN1_SOURCE_DIR}/dll_plugin1.cpp")
  message(FATAL_ERROR "dll_plugin1 source not found: ${DLL_PLUGIN1_SOURCE_DIR}")
endif()

# Release-only: dll_plugin1 is a test helper and debug build is not needed
set(VCPKG_BUILD_TYPE release)
set(VCPKG_POLICY_DLLS_IN_STATIC_LIBRARY enabled)
set(VCPKG_POLICY_DLLS_WITHOUT_LIBS enabled)
set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)
set(VCPKG_POLICY_MISMATCHED_NUMBER_OF_BINARIES enabled)

vcpkg_cmake_configure(
  SOURCE_PATH "${DLL_PLUGIN1_SOURCE_DIR}"
)

vcpkg_cmake_install()

file(MAKE_DIRECTORY "${CURRENT_PACKAGES_DIR}/share/${PORT}")

file(WRITE "${CURRENT_PACKAGES_DIR}/share/${PORT}/dll-plugin1Config.cmake"
  "# dll-plugin1: test helper DLL for sakura editor\n"
)

file(WRITE "${CURRENT_PACKAGES_DIR}/share/${PORT}/dll-plugin1ConfigVersion.cmake" [[
set(PACKAGE_VERSION "1.0.0")
if(PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION)
  set(PACKAGE_VERSION_COMPATIBLE FALSE)
else()
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
  if(PACKAGE_FIND_VERSION STREQUAL PACKAGE_VERSION)
    set(PACKAGE_VERSION_EXACT TRUE)
  endif()
endif()
]])

configure_file("${CMAKE_CURRENT_LIST_DIR}/usage" "${CURRENT_PACKAGES_DIR}/share/${PORT}/usage" COPYONLY)

file(INSTALL
  "${REPO_ROOT}/LICENSE"
  DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
  RENAME copyright
)
