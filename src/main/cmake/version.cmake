# version.cmake - Generate version.h at build time

# Function to generate simple conditional define
function(generate_simple_define var_name define_name)
  if("${${var_name}}" STREQUAL "")
    set(${define_name}_DEFINE "// ${define_name} is not defined" PARENT_SCOPE)
  else()
    set(${define_name}_DEFINE "#define ${define_name} \"${${var_name}}\"" PARENT_SCOPE)
  endif()
endfunction()

# Function to generate build number type defines (with INT and LABEL variants)
function(generate_build_number_define var_name base_name label_prefix)
  if("${${var_name}}" STREQUAL "")
    set(${base_name}_DEFINE "// ${base_name}     is not defined\n// ${base_name}_INT is not defined\n// ${base_name}_LABEL is not defined" PARENT_SCOPE)
  else()
    set(${base_name}_DEFINE "#define ${base_name}             \"${${var_name}}\"\n#define ${base_name}_INT          ${${var_name}}\n#define ${base_name}_LABEL \"${label_prefix} ${${var_name}}\"" PARENT_SCOPE)
  endif()
endfunction()

# Function to generate URL type defines (with specific formatting)
function(generate_url_define var_name define_name)
  if("${${var_name}}" STREQUAL "")
    set(${define_name}_DEFINE "// ${define_name} is not defined" PARENT_SCOPE)
  else()
    # Adjust spacing for alignment
    string(LENGTH "${define_name}" name_length)
    if(name_length LESS 25)
      math(EXPR spaces_needed "25 - ${name_length}")
      string(REPEAT " " ${spaces_needed} spacing)
    else()
      set(spacing " ")
    endif()
    set(${define_name}_DEFINE "#define ${define_name}${spacing}\"${${var_name}}\"" PARENT_SCOPE)
  endif()
endfunction()

# Find Git with additional search paths
find_program(GIT_EXECUTABLE git
  PATHS
    "$ENV{ProgramFiles}/Git"
  PATH_SUFFIXES
    cmd
    bin
)

# Debug: Print Git executable path
if(GIT_EXECUTABLE)
  message(STATUS "Found Git: ${GIT_EXECUTABLE}")
else()
  message(STATUS "Git not found")
endif()

# Initialize variables with default values
set(BUILD_VERSION "0")

# Get git information if Git is available
if(GIT_EXECUTABLE AND EXISTS "${SOURCE_DIR}/.git")
  message(STATUS "Git repository detected, extracting version information...")
  
  # Get remote origin URL
  execute_process(
    COMMAND ${GIT_EXECUTABLE} config --get remote.origin.url
    WORKING_DIRECTORY ${SOURCE_DIR}
    OUTPUT_VARIABLE GIT_REMOTE_ORIGIN_URL
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  
  # Get full commit hash
  execute_process(
    COMMAND ${GIT_EXECUTABLE} show -s --format=%H
    WORKING_DIRECTORY ${SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  
  # Get short commit hash
  string(SUBSTRING "${GIT_COMMIT_HASH}" 0 8 GIT_SHORT_COMMIT_HASH)
  
  # Get commit count (build version)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-list --count --no-merges @
    WORKING_DIRECTORY ${SOURCE_DIR}
    OUTPUT_VARIABLE BUILD_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  
else()
  message(STATUS "NOTE: No .git directory found")
endif()

# If BUILD_VERSION is empty, set it to 0
if("${BUILD_VERSION}" STREQUAL "")
  set(BUILD_VERSION "0")
endif()

if(DEFINED ENV{GITHUB_ACTIONS} AND "$ENV{GITHUB_ACTIONS}" STREQUAL "true")
  set(BUILD_ENV_NAME "GHA")

  set(CI_BUILD_NUMBER "$ENV{GITHUB_RUN_NUMBER}")
  set(CI_BUILD_URL "$ENV{GITHUB_SERVER_URL}/$ENV{GITHUB_REPOSITORY}/actions/runs/$ENV{GITHUB_RUN_ID}")
  set(GITHUB_COMMIT_URL "$ENV{GITHUB_SERVER_URL}/$ENV{GITHUB_REPOSITORY}/commit/$ENV{GITHUB_SHA}")

  if ("$ENV{GITHUB_EVENT_NAME}" STREQUAL "pull_request")
    string(REGEX REPLACE "^([0-9]+)/merge$" "\\1" GITHUB_PR_NUMBER "$ENV{GITHUB_REF_NAME}")
    set(GITHUB_PR_HEAD_COMMIT "$ENV{GITHUB_SHA}")
    string(SUBSTRING "${GITHUB_PR_HEAD_COMMIT}" 0 8 GITHUB_PR_HEAD_SHORT_COMMIT)
    set(GITHUB_PR_HEAD_URL "$ENV{GITHUB_SERVER_URL}/$ENV{GITHUB_REPOSITORY}/pull/${GITHUB_PR_NUMBER}/commits/${GITHUB_PR_HEAD_COMMIT}")
  endif()

  if("$ENV{GITHUB_REF_NAME}" STREQUAL "tag")
    set(GITHUB_TAG_NAME "$ENV{GITHUB_REF_NAME}")
  endif()
else()
  set(BUILD_ENV_NAME "Local")
endif()

# Generate conditional defines for version.h.in
generate_simple_define(GIT_SHORT_COMMIT_HASH    GIT_SHORT_COMMIT_HASH)
generate_simple_define(GIT_COMMIT_HASH          GIT_COMMIT_HASH)
generate_simple_define(GIT_REMOTE_ORIGIN_URL    GIT_REMOTE_ORIGIN_URL)
generate_simple_define(GIT_TAG_NAME             GIT_TAG_NAME)

generate_build_number_define(CI_BUILD_NUMBER    CI_BUILD_NUMBER "Build")
generate_build_number_define(GITHUB_PR_NUMBER   GITHUB_PR_NUMBER "PR")

generate_url_define(GITHUB_COMMIT_URL           GITHUB_COMMIT_URL)
generate_url_define(GITHUB_PR_HEAD_URL          GITHUB_PR_HEAD_URL)
generate_url_define(GITHUB_PR_HEAD_COMMIT       GITHUB_PR_HEAD_COMMIT)
generate_url_define(GITHUB_PR_HEAD_SHORT_COMMIT GITHUB_PR_HEAD_SHORT_COMMIT)
generate_url_define(CI_BUILD_URL                CI_BUILD_URL)

# Configure the version.h file
configure_file(
  ${SOURCE_DIR}/src/main/cmake/version.h.in
  ${BINARY_DIR}/version.h
  @ONLY
)

# Print information (like the original batch file)
message(STATUS "GIT_REMOTE_ORIGIN_URL       : ${GIT_REMOTE_ORIGIN_URL}")
message(STATUS "GIT_COMMIT_HASH             : ${GIT_COMMIT_HASH}")
message(STATUS "GIT_SHORT_COMMIT_HASH       : ${GIT_SHORT_COMMIT_HASH}")
message(STATUS "")
message(STATUS "BUILD_ENV_NAME              : ${BUILD_ENV_NAME}")
message(STATUS "BUILD_VERSION               : ${BUILD_VERSION}")
message(STATUS "")
message(STATUS "GITHUB_COMMIT_URL           : ${GITHUB_COMMIT_URL}")
message(STATUS "CI_BUILD_NUMBER             : ${CI_BUILD_NUMBER}")
message(STATUS "CI_BUILD_URL                : ${CI_BUILD_URL}")
message(STATUS "GITHUB_PR_HEAD_URL          : ${GITHUB_PR_HEAD_URL}")
message(STATUS "GITHUB_PR_NUMBER            : ${GITHUB_PR_NUMBER}")
message(STATUS "GITHUB_PR_HEAD_COMMIT       : ${GITHUB_PR_HEAD_COMMIT}")
message(STATUS "GITHUB_PR_HEAD_SHORT_COMMIT : ${GITHUB_PR_HEAD_SHORT_COMMIT}")
message(STATUS "")
message(STATUS "GIT_TAG_NAME                : ${GIT_TAG_NAME}")
message(STATUS "")
message(STATUS "version.h was updated.")
