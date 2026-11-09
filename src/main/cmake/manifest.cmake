# manifest.cmake - Generate manifest at build time
#
# arguments(required):
#   ${SOURCE_DIR}
#   ${OUTPUT_FILE}
#   ${EXE_ARCH}
#   ${EXE_NAME}
#

# Configure the file
configure_file(
  ${SOURCE_DIR}/src/main/cmake/manifest.in
  ${OUTPUT_FILE}
  @ONLY
)

