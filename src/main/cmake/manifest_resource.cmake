# manifest_resource.cmake - Generate manifest_resource at build time
#
# arguments(required):
#   ${SOURCE_DIR}
#   ${OUTPUT_FILE}
#   ${MANIFEST_FILE}
#

# Configure the file
configure_file(
  ${SOURCE_DIR}/src/main/cmake/manifest_resource.in
  ${OUTPUT_FILE}
  @ONLY
)
