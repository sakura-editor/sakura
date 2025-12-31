# CMake script for ChmSourceConverter
#
# requires
#   ${CMD_VSWHERE}
#   ${GENERATOR_ARGS_FOR_HOST_TOOLS}

# Define the ChmSourceConverter's path
set(CHM_SOURCE_CONVERTER_BUILD_DIR "${CMAKE_BINARY_DIR}/ChmSourceConverter")
set(CHM_SOURCE_CONVERTER ${CMAKE_BINARY_DIR}/ChmSourceConverter/bin/$<CONFIG>/ChmSourceConverter.exe)
set(CHM_SOURCE_CONVERTER_EXECUTABLE "${CMAKE_BINARY_DIR}/bin/ChmSourceConverter.exe")

message(STATUS "ChmSourceConverter config: cmake -G \"${CMAKE_GENERATOR}\" ${GENERATOR_ARGS_FOR_HOST_TOOLS} -S \"${CMAKE_SOURCE_DIR}/tools/ChmSourceConverter\" -B \"${CHM_SOURCE_CONVERTER_BUILD_DIR}\" -DCMD_VSWHERE=\"${CMD_VSWHERE}\" \"-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}\"")

add_custom_command(
  OUTPUT "${CHM_SOURCE_CONVERTER_BUILD_DIR}/CMakeCache.txt"
  COMMAND ${CMAKE_COMMAND}
    -G "${CMAKE_GENERATOR}"
    ${GENERATOR_ARGS_FOR_HOST_TOOLS}
    -S "${CMAKE_SOURCE_DIR}/tools/ChmSourceConverter"
    -B "${CHM_SOURCE_CONVERTER_BUILD_DIR}"
    -DCMD_VSWHERE="${CMD_VSWHERE}"
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}"
  COMMENT "Configure ChmSourceConverter Command"
)

add_custom_target(configure_chm_source_converter
  DEPENDS
    "${CHM_SOURCE_CONVERTER_BUILD_DIR}/CMakeCache.txt"
)

add_custom_command(
  OUTPUT "${CHM_SOURCE_CONVERTER}"
  COMMAND ${CMAKE_COMMAND} --build "${CHM_SOURCE_CONVERTER_BUILD_DIR}" --config $<CONFIG>
  DEPENDS configure_chm_source_converter
  COMMENT "Building ChmSourceConverter Command"
)

add_custom_target(build_chm_source_converter
  DEPENDS
    "${CHM_SOURCE_CONVERTER}"
)

add_custom_command(
  OUTPUT "${CHM_SOURCE_CONVERTER_EXECUTABLE}"
  COMMAND ${CMAKE_COMMAND} --install "${CHM_SOURCE_CONVERTER_BUILD_DIR}" --config $<CONFIG>
  DEPENDS build_chm_source_converter
  COMMENT "Installing ChmSourceConverter Command"
)

add_custom_target(generate_chm_source_converter
  DEPENDS
    "${CHM_SOURCE_CONVERTER_EXECUTABLE}"
)
