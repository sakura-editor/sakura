# CMake script for HeaderMake
#
# requires
#   ${CMAKE_GENERATOR}
#   ${GENERATOR_ARGS_FOR_HOST_TOOLS}
#   ${HOST_PLATFORM}

# Define the HeaderMake's path
set(HEADER_MAKE_BUILD_DIR "${CMAKE_BINARY_DIR}/HeaderMake")
set(HEADER_MAKE_EXECUTABLE "${CMAKE_BINARY_DIR}/bin/HeaderMake.exe")

message(STATUS "HeaderMake config: cmake -G \"${CMAKE_GENERATOR}\" ${GENERATOR_ARGS_FOR_HOST_TOOLS} -S \"${CMAKE_SOURCE_DIR}/tools/HeaderMake\" -B \"${HEADER_MAKE_BUILD_DIR}\" \"-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}\"")

add_custom_command(
  OUTPUT "${HEADER_MAKE_BUILD_DIR}/CMakeCache.txt"
  COMMAND ${CMAKE_COMMAND}
    -G "${CMAKE_GENERATOR}"
    ${GENERATOR_ARGS_FOR_HOST_TOOLS}
    -S "${CMAKE_SOURCE_DIR}/tools/HeaderMake"
    -B "${HEADER_MAKE_BUILD_DIR}"
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}"
  COMMENT "Configure HeaderMake Command"
)

add_custom_target(configure_header_make
  DEPENDS
    "${HEADER_MAKE_BUILD_DIR}/CMakeCache.txt"
)

add_custom_command(
  OUTPUT "${HEADER_MAKE_EXECUTABLE}"
  COMMAND ${CMAKE_COMMAND} --build "${HEADER_MAKE_BUILD_DIR}" --config $<CONFIG>
  COMMAND ${CMAKE_COMMAND} --install "${HEADER_MAKE_BUILD_DIR}" --config $<CONFIG>
  DEPENDS configure_header_make
  COMMENT "Building HeaderMake Command"
)

add_custom_target(generate_header_make
  DEPENDS
    "${HEADER_MAKE_EXECUTABLE}"
)
