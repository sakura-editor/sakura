get_filename_component(REPO_ROOT "${CURRENT_PORT_DIR}/../../../.." ABSOLUTE)
set(BREGONIG_SOURCE_DIR "${REPO_ROOT}/externals/bregonig")

if(NOT EXISTS "${BREGONIG_SOURCE_DIR}/src/bregexp.h")
  message(FATAL_ERROR "bregonig source not found: ${BREGONIG_SOURCE_DIR}")
endif()

# Map vcpkg architecture to bregonig TARGET_CPU
if(VCPKG_TARGET_ARCHITECTURE STREQUAL "x64")
  set(TARGET_CPU "x64")
elseif(VCPKG_TARGET_ARCHITECTURE STREQUAL "x86")
  set(TARGET_CPU "x86")
else()
  message(FATAL_ERROR "Unsupported architecture: ${VCPKG_TARGET_ARCHITECTURE}")
endif()

# Find vswhere and VsDevCmd.bat
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

set(VCPKG_POLICY_DLLS_WITHOUT_LIBS enabled)
set(VCPKG_POLICY_DLLS_IN_STATIC_LIBRARY enabled)

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

# Copy source to build tree
set(BUILD_DIR "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}")
file(REMOVE_RECURSE "${BUILD_DIR}")
file(COPY "${BREGONIG_SOURCE_DIR}/" DESTINATION "${BUILD_DIR}")

# Apply DYNAMICBASE tweak directly (avoid git-context-dependent patching)
set(BREGONIG_MAKEFILE "${BUILD_DIR}/src/Makefile")
if(NOT EXISTS "${BREGONIG_MAKEFILE}")
  message(FATAL_ERROR "Makefile not found: ${BREGONIG_MAKEFILE}")
endif()
file(READ "${BREGONIG_MAKEFILE}" BREGONIG_MAKEFILE_CONTENT)
string(REPLACE
  "LDFLAGS = /DLL /nologo /MAP /BASE:$(BASEADDR) /merge:.rdata=.text"
  "LDFLAGS = /DLL /nologo /MAP /DYNAMICBASE /merge:.rdata=.text"
  BREGONIG_MAKEFILE_CONTENT
  "${BREGONIG_MAKEFILE_CONTENT}"
)
file(WRITE "${BREGONIG_MAKEFILE}" "${BREGONIG_MAKEFILE_CONTENT}")

# Prepare onigmo dependency (in vcpkg_installed)
set(ONIGMO_INSTALLED "${CURRENT_INSTALLED_DIR}")
if(NOT EXISTS "${ONIGMO_INSTALLED}/include/onigmo.h")
  message(FATAL_ERROR "onigmo not installed: ${ONIGMO_INSTALLED}/include/onigmo.h")
endif()

# Create onigmo directory structure that Makefile expects
file(MAKE_DIRECTORY "${BUILD_DIR}/src/onigmo/build_${TARGET_CPU}" "${BUILD_DIR}/src/onigmo/build_${TARGET_CPU}d")

# Copy headers
file(COPY "${ONIGMO_INSTALLED}/include/onigmo.h" DESTINATION "${BUILD_DIR}/src/onigmo/")

# ---- Release build ----
file(COPY "${ONIGMO_INSTALLED}/lib/onigmo.lib" 
  DESTINATION "${BUILD_DIR}/src/onigmo/build_${TARGET_CPU}/"
  NO_SOURCE_PERMISSIONS)
# Rename to onigmo_s.lib as Makefile expects
file(RENAME
  "${BUILD_DIR}/src/onigmo/build_${TARGET_CPU}/onigmo.lib"
  "${BUILD_DIR}/src/onigmo/build_${TARGET_CPU}/onigmo_s.lib"
)

file(WRITE "${BUILD_DIR}/build_rel.cmd"
  "@echo off\n"
  "call \"${VSDEVCMD_PATH}\" -host_arch=x64 -arch=${TARGET_CPU}\n"
  "if errorlevel 1 exit /b 1\n"
  "nmake ONIG_DIR=onigmo TARGET_CPU=${TARGET_CPU}\n"
)
vcpkg_execute_required_process(
  COMMAND cmd.exe /c "${BUILD_DIR}/build_rel.cmd"
  WORKING_DIRECTORY "${BUILD_DIR}/src"
  LOGNAME "build-${TARGET_TRIPLET}-rel"
)

# ---- Debug build ----
file(COPY "${ONIGMO_INSTALLED}/debug/lib/onigmo.lib"
  DESTINATION "${BUILD_DIR}/src/onigmo/build_${TARGET_CPU}d/"
  NO_SOURCE_PERMISSIONS)
# Rename to onigmo_s.lib as Makefile expects
file(RENAME
  "${BUILD_DIR}/src/onigmo/build_${TARGET_CPU}d/onigmo.lib"
  "${BUILD_DIR}/src/onigmo/build_${TARGET_CPU}d/onigmo_s.lib"
)

file(WRITE "${BUILD_DIR}/build_dbg.cmd"
  "@echo off\n"
  "call \"${VSDEVCMD_PATH}\" -host_arch=x64 -arch=${TARGET_CPU}\n"
  "if errorlevel 1 exit /b 1\n"
  "nmake ONIG_DIR=onigmo TARGET_CPU=${TARGET_CPU} DEBUG=1\n"
)
vcpkg_execute_required_process(
  COMMAND cmd.exe /c "${BUILD_DIR}/build_dbg.cmd"
  WORKING_DIRECTORY "${BUILD_DIR}/src"
  LOGNAME "build-${TARGET_TRIPLET}-dbg"
)

# ---- Verify outputs ----
set(REL_DLL "${BUILD_DIR}/src/obj${TARGET_CPU}/bregonig.dll")
set(DBG_DLL "${BUILD_DIR}/src/obj${TARGET_CPU}d/bregonig.dll")
if(NOT EXISTS "${REL_DLL}")
  message(FATAL_ERROR "Release DLL not found: ${REL_DLL}")
endif()
if(NOT EXISTS "${DBG_DLL}")
  message(FATAL_ERROR "Debug DLL not found: ${DBG_DLL}")
endif()

# ---- Install files ----
file(MAKE_DIRECTORY "${CURRENT_PACKAGES_DIR}/bin" "${CURRENT_PACKAGES_DIR}/debug/bin" "${CURRENT_PACKAGES_DIR}/include")

file(INSTALL "${REL_DLL}" DESTINATION "${CURRENT_PACKAGES_DIR}/bin")
file(INSTALL "${DBG_DLL}" DESTINATION "${CURRENT_PACKAGES_DIR}/debug/bin")
file(INSTALL "${BUILD_DIR}/src/bregexp.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include")

# ---- CMake config ----
file(MAKE_DIRECTORY "${CURRENT_PACKAGES_DIR}/share/bregonig")
file(WRITE "${CURRENT_PACKAGES_DIR}/share/bregonig/bregonigConfig.cmake" [[
if(NOT TARGET bregonig::bregonig)
  get_filename_component(_BREGONIG_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
  add_library(bregonig::bregonig SHARED IMPORTED)
  set_target_properties(bregonig::bregonig PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES   "${_BREGONIG_ROOT}/include"
    IMPORTED_CONFIGURATIONS         "RELEASE;DEBUG"
    IMPORTED_LOCATION_RELEASE       "${_BREGONIG_ROOT}/bin/bregonig.dll"
    IMPORTED_IMPLIB_RELEASE         "${_BREGONIG_ROOT}/lib/bregonig.lib"
    IMPORTED_LOCATION_DEBUG         "${_BREGONIG_ROOT}/debug/bin/bregonig.dll"
    IMPORTED_IMPLIB_DEBUG           "${_BREGONIG_ROOT}/debug/lib/bregonig.lib"
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE
    MAP_IMPORTED_CONFIG_MINSIZEREL     RELEASE
  )
endif()
]])

file(WRITE "${CURRENT_PACKAGES_DIR}/share/bregonig/bregonigConfigVersion.cmake" [[
set(PACKAGE_VERSION "4.20.0")
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
if(EXISTS "${BREGONIG_SOURCE_DIR}/bsd_license.txt")
  file(INSTALL
    "${BREGONIG_SOURCE_DIR}/bsd_license.txt"
    DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
    RENAME copyright
  )
elseif(EXISTS "${BREGONIG_SOURCE_DIR}/README.md")
  file(INSTALL
    "${BREGONIG_SOURCE_DIR}/README.md"
    DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
    RENAME copyright
  )
endif()
