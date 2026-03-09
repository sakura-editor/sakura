# CMake script for DarkModeLib
#
# requires:
#   ${CMAKE_GENERATOR}
#   ${GENERATOR_ARGS}

# Define the DarkModeLib's path
set(DARKMODE_LIB_BUILD_DIR "${CMAKE_BINARY_DIR}/DarkModeLib")
set(DARKMODE_LIB_PC "${CMAKE_BINARY_DIR}/lib$<$<CONFIG:Debug>:/Debug>/pkgconfig/darkmodelib.pc")

message(STATUS "DarkModeLib config: cmake -G \"${CMAKE_GENERATOR}\" ${GENERATOR_ARGS} -S \"${CMAKE_SOURCE_DIR}/externals/darkmodelib\" -B \"${DARKMODE_LIB_BUILD_DIR}\" \"-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}\" ${GENERATOR_ARGS_FOR_STATIC_LIBRARY}")

add_custom_command(
  OUTPUT
    "${CMAKE_SOURCE_DIR}/externals/darkmodelib/.git"
  COMMAND ${CMAKE_COMMAND}
    -DGIT_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
    -DREPO_ROOT:PATH=${CMAKE_SOURCE_DIR}
    -DSUBMODULE_PATH:STRING=externals/darkmodelib
    -DLOCK_PATH:FILEPATH=${CMAKE_BINARY_DIR}/cmake-submodule-update.lock
    -P ${CMAKE_SOURCE_DIR}/src/main/cmake/git_submodule_update_locked.cmake
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  COMMENT "Fetching DarkModeLib's source files"
)

add_custom_target(fetch_darkmodelib_source_files
  DEPENDS
    "${CMAKE_SOURCE_DIR}/externals/darkmodelib/.git"
)

add_custom_command(
  OUTPUT "${DARKMODE_LIB_BUILD_DIR}/CMakeCache.txt"
  COMMAND ${CMAKE_COMMAND}
    -G "${CMAKE_GENERATOR}"
    ${GENERATOR_ARGS}
    -S "${CMAKE_SOURCE_DIR}/tools/darkmodelib"
    -B "${DARKMODE_LIB_BUILD_DIR}"
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}"
    "-DCMAKE_INSTALL_LIBDIR=lib$<$<CONFIG:Debug>:/Debug>"
    ${GENERATOR_ARGS_FOR_STATIC_LIBRARY}
  DEPENDS fetch_darkmodelib_source_files
  COMMENT "Configure DarkModeLib Library"
)

add_custom_target(configure_darkmodelib
  DEPENDS
    "${DARKMODE_LIB_BUILD_DIR}/CMakeCache.txt"
)

add_custom_command(
  OUTPUT "${DARKMODE_LIB_PC}"
  COMMAND ${CMAKE_COMMAND} --build "${DARKMODE_LIB_BUILD_DIR}" --config $<CONFIG>
  COMMAND ${CMAKE_COMMAND} --install "${DARKMODE_LIB_BUILD_DIR}" --config $<CONFIG>
  DEPENDS configure_darkmodelib
  COMMENT "Building DarkModeLib Library"
)

add_custom_target(generate_darkmodelib
  DEPENDS
    "${DARKMODE_LIB_PC}"
)
