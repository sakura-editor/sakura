get_filename_component(REPO_ROOT "${CURRENT_PORT_DIR}/../../../.." ABSOLUTE)
set(ONIGMO_SOURCE_DIR "${REPO_ROOT}/externals/Onigmo")

if(NOT EXISTS "${ONIGMO_SOURCE_DIR}/onigmo.h")
  message(FATAL_ERROR "Onigmo source not found: ${ONIGMO_SOURCE_DIR}")
endif()

# Map vcpkg architecture to Onigmo ARCH
if(VCPKG_TARGET_ARCHITECTURE STREQUAL "x64")
  set(ONIGMO_ARCH "x64")
elseif(VCPKG_TARGET_ARCHITECTURE STREQUAL "x86")
  set(ONIGMO_ARCH "x86")
else()
  message(FATAL_ERROR "Unsupported architecture: ${VCPKG_TARGET_ARCHITECTURE}")
endif()

# Find vswhere and VsDevCmd.bat
# $ENV{ProgramFiles(x86)} is invalid in CMake (parentheses), so use cmd to resolve it.
execute_process(
  COMMAND cmd.exe /c "echo %ProgramFiles(x86)%"
  OUTPUT_VARIABLE _PROG_FILES_X86
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
find_program(VSWHERE vswhere
  HINTS
    "${_PROG_FILES_X86}/Microsoft Visual Studio/Installer"
    "$ENV{ProgramFiles}/Microsoft Visual Studio/Installer"
)
if(NOT VSWHERE)
  message(FATAL_ERROR "vswhere not found. Install Visual Studio 2017 or later.")
endif()

execute_process(
  COMMAND "${VSWHERE}" -latest -find "Common7/Tools/VsDevCmd.bat"
  OUTPUT_VARIABLE VSDEVCMD_PATH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE VSWHERE_RESULT
)
if(NOT VSWHERE_RESULT EQUAL 0 OR NOT EXISTS "${VSDEVCMD_PATH}")
  message(FATAL_ERROR "VsDevCmd.bat not found.")
endif()
string(REPLACE "/" "\\" VSDEVCMD_PATH "${VSDEVCMD_PATH}")

# Copy source to build tree (nmake builds in-place)
set(BUILD_DIR "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}")
file(REMOVE_RECURSE "${BUILD_DIR}")
file(COPY "${ONIGMO_SOURCE_DIR}/" DESTINATION "${BUILD_DIR}")

# ---- Release build ----
file(WRITE "${BUILD_DIR}/build_rel.cmd"
  "@echo off\n"
  "call \"${VSDEVCMD_PATH}\" -host_arch=x64 -arch=${ONIGMO_ARCH}\n"
  "if errorlevel 1 exit /b 1\n"
  "call build_nmake.cmd ARCH=${ONIGMO_ARCH}\n"
)
vcpkg_execute_required_process(
  COMMAND cmd.exe /c build_rel.cmd
  WORKING_DIRECTORY "${BUILD_DIR}"
  LOGNAME "build-${TARGET_TRIPLET}-rel"
)

# ---- Debug build ----
file(WRITE "${BUILD_DIR}/build_dbg.cmd"
  "@echo off\n"
  "call \"${VSDEVCMD_PATH}\" -host_arch=x64 -arch=${ONIGMO_ARCH}\n"
  "if errorlevel 1 exit /b 1\n"
  "call build_nmake.cmd ARCH=${ONIGMO_ARCH} DEBUG=1\n"
)
vcpkg_execute_required_process(
  COMMAND cmd.exe /c build_dbg.cmd
  WORKING_DIRECTORY "${BUILD_DIR}"
  LOGNAME "build-${TARGET_TRIPLET}-dbg"
)

# ---- Verify outputs ----
set(REL_LIB "${BUILD_DIR}/build_${ONIGMO_ARCH}/onigmo_s.lib")
set(DBG_LIB "${BUILD_DIR}/build_${ONIGMO_ARCH}d/onigmo_s.lib")
if(NOT EXISTS "${REL_LIB}")
  message(FATAL_ERROR "Release library not found: ${REL_LIB}")
endif()
if(NOT EXISTS "${DBG_LIB}")
  message(FATAL_ERROR "Debug library not found: ${DBG_LIB}")
endif()

# ---- Install headers ----
file(INSTALL
  "${ONIGMO_SOURCE_DIR}/onigmo.h"
  "${ONIGMO_SOURCE_DIR}/onigmognu.h"
  "${ONIGMO_SOURCE_DIR}/onigmoposix.h"
  DESTINATION "${CURRENT_PACKAGES_DIR}/include"
)

# ---- Install libraries ----
file(INSTALL "${REL_LIB}" DESTINATION "${CURRENT_PACKAGES_DIR}/lib"       RENAME "onigmo.lib")
file(INSTALL "${DBG_LIB}" DESTINATION "${CURRENT_PACKAGES_DIR}/debug/lib" RENAME "onigmo.lib")

# ---- CMake config ----
file(MAKE_DIRECTORY "${CURRENT_PACKAGES_DIR}/share/onigmo")
file(WRITE "${CURRENT_PACKAGES_DIR}/share/onigmo/OnigmoConfig.cmake" [[
if(NOT TARGET Onigmo::onigmo)
  get_filename_component(_ONIGMO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
  add_library(Onigmo::onigmo STATIC IMPORTED)
  set_target_properties(Onigmo::onigmo PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES   "${_ONIGMO_ROOT}/include"
    IMPORTED_CONFIGURATIONS         "RELEASE;DEBUG"
    IMPORTED_LOCATION_RELEASE       "${_ONIGMO_ROOT}/lib/onigmo.lib"
    IMPORTED_LOCATION_DEBUG         "${_ONIGMO_ROOT}/debug/lib/onigmo.lib"
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE
    MAP_IMPORTED_CONFIG_MINSIZEREL     RELEASE
  )
endif()
]])

file(WRITE "${CURRENT_PACKAGES_DIR}/share/onigmo/OnigmoConfigVersion.cmake" [[
set(PACKAGE_VERSION "6.2.0")
if(PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION)
  set(PACKAGE_VERSION_COMPATIBLE FALSE)
else()
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
  if(PACKAGE_FIND_VERSION STREQUAL PACKAGE_VERSION)
    set(PACKAGE_VERSION_EXACT TRUE)
  endif()
endif()
]])

# ---- Usage ----
configure_file("${CMAKE_CURRENT_LIST_DIR}/usage" "${CURRENT_PACKAGES_DIR}/share/${PORT}/usage" COPYONLY)

# ---- Copyright ----
file(INSTALL
  "${ONIGMO_SOURCE_DIR}/COPYING"
  DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
  RENAME copyright
)
