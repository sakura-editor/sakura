# CMake script for HeaderMake
#
# requires:
#   BUILD_PLA${GENERATOR_ARGS}TFORM

# Define the compiletests's path
set(COMPILE_TEST_BUILD_DIR "${CMAKE_BINARY_DIR}/compiletests")
set(COMPILE_TEST_CACHE "${COMPILE_TEST_BUILD_DIR}/compiletests/CMakeCache.txt")

add_custom_command(
  OUTPUT "${COMPILE_TEST_CACHE}"
  COMMAND ${CMAKE_COMMAND}
    -G "${CMAKE_GENERATOR}"
    ${GENERATOR_ARGS}
    -S "${CMAKE_SOURCE_DIR}/src/test/resources/compiletests"
    -B "${COMPILE_TEST_BUILD_DIR}"
  COMMENT "Running compiletests"
)

add_custom_target(run_compiletests
  DEPENDS
    "${COMPILE_TEST_CACHE}"
)
