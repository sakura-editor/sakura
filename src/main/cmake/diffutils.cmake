# CMake script for diffutils
#
# requires
#   ${7ZIP_EXECUTABLE}

# Define the ctags's path
set(DIFF_VERSION "2.8.7-1")
set(DIFF_ZIP_FILE1 "${CMAKE_SOURCE_DIR}/externals/diffutils/diffutils-${DIFF_VERSION}-bin.zip")
set(DIFF_ZIP_FILE2 "${CMAKE_SOURCE_DIR}/externals/diffutils/diffutils-${DIFF_VERSION}-dep.zip")
set(DIFF_EXECUTABLE "${OUTPUT_DIRECTORY}/diff.exe")

add_custom_command(
  OUTPUT "${DIFF_EXECUTABLE}"
  COMMAND ${7ZIP_EXECUTABLE} e "${DIFF_ZIP_FILE1}" -o"${OUTPUT_DIRECTORY}" -y bin/diff.exe > NUL
  COMMAND ${7ZIP_EXECUTABLE} e "${DIFF_ZIP_FILE2}" -o"${OUTPUT_DIRECTORY}" -y bin/libintl3.dll bin/libiconv2.dll > NUL
  COMMENT "Extracting ctags.exe from ${DIFF_ZIP_FILE}"
)

add_custom_target(generate_diffutils ALL
  DEPENDS
    "${DIFF_EXECUTABLE}"
)
