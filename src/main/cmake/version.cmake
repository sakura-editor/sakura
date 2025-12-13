# version.cmake - Generate version.h at build time
#
# arguments(required):
#   ${SOURCE_DIR}
#   ${GIT_EXECUTABLE}
#
# Environment variables (optional):
#   $ENV{GITHUB_ACTIONS}
#   $ENV{GITHUB_SERVER_URL}
#   $ENV{GITHUB_REPOSITORY}
#   $ENV{GITHUB_SHA}
#   $ENV{GITHUB_RUN_NUMBER}
#   $ENV{GITHUB_RUN_ID}
#   $ENV{GITHUB_REF_NAME}
#   $ENV{GITHUB_REF_TYPE}

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

# Initialize variables with default values
set(SAKURA_MAJOR_VERSION "2") # メジャーバージョン(2固定)
set(SAKURA_MINOR_VERSION "4") # マイナーバージョン(4以降はGitHub版)
set(SAKURA_PATCH_VERSION "3") # 連番(マージの通し番号)
set(BUILD_VERSION "0")

# CIのビルドページURL(ENVで指定する)
if(DEFINED ENV{CI_BUILD_URL})
  set(CI_BUILD_URL "$ENV{CI_BUILD_URL}")
endif()

# CIのビルド番号(ENVで指定する)
if(DEFINED ENV{CI_BUILD_NUMBER})
  set(CI_BUILD_NUMBER "$ENV{GITHUB_RUN_NUMBER}")
endif()

# GitHub Actionsで実行されてる場合の処理
if("$ENV{GITHUB_ACTIONS}" STREQUAL "true")
  set(BUILD_ENV_NAME "GHA")

  if ("$ENV{GITHUB_EVENT_NAME}" STREQUAL "pull_request")
    set(GITHUB_PR_NUMBER "$ENV{GITHUB_PR_NUMBER}")
    set(GITHUB_PR_HEAD_URL "$ENV{GITHUB_PR_HEAD_URL}")
    set(GITHUB_PR_HEAD_COMMIT "$ENV{GITHUB_PR_HEAD_COMMIT}")
    set(GITHUB_PR_HEAD_SHORT_COMMIT "$ENV{GITHUB_PR_HEAD_SHORT_COMMIT}")
  elseif("$ENV{GITHUB_REF_TYPE}" STREQUAL "tag")
    set(GITHUB_TAG_NAME "$ENV{GITHUB_REF_NAME}")
  endif()

  set(GIT_REMOTE_ORIGIN_URL "$ENV{GITHUB_SERVER_URL}/$ENV{GITHUB_REPOSITORY}")
  set(GIT_COMMIT_HASH "$ENV{GITHUB_SHA}")
  set(GITHUB_COMMIT_URL "$ENV{GITHUB_COMMIT_URL}")

else()
  set(BUILD_ENV_NAME "Local")

  # Get git information if Git is available
  if(IS_DIRECTORY "${SOURCE_DIR}/.git")
    message(STATUS "Git repository detected, extracting version information...")
    
    # Get remote origin URL
    execute_process(
      COMMAND ${GIT_EXECUTABLE} config --get remote.origin.url
      WORKING_DIRECTORY "${SOURCE_DIR}"
      OUTPUT_VARIABLE GIT_REMOTE_ORIGIN_URL
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
    if(GIT_REMOTE_ORIGIN_URL MATCHES "^git@github.com:.+")
      string(REGEX REPLACE "^git@github.com:(.+)(\\.git)" "https://github.com/\\1" GIT_REMOTE_ORIGIN_URL "${GIT_REMOTE_ORIGIN_URL}")
    endif()

    # Get full commit hash
    execute_process(
      COMMAND ${GIT_EXECUTABLE} show -s --format=%H
      WORKING_DIRECTORY "${SOURCE_DIR}"
      OUTPUT_VARIABLE GIT_COMMIT_HASH
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
  else()
    message(STATUS "Git repository not detected.")
  endif()
endif()

# Get short commit hash
string(SUBSTRING "${GIT_COMMIT_HASH}" 0 8 GIT_SHORT_COMMIT_HASH)

if(IS_DIRECTORY "${SOURCE_DIR}/.git")
  # Get commit count (build version)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-list --count --no-merges @
    WORKING_DIRECTORY ${SOURCE_DIR}
    OUTPUT_VARIABLE BUILD_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
endif()

# If BUILD_VERSION is empty, set it to 0
if("${BUILD_VERSION}" STREQUAL "")
  set(BUILD_VERSION "0")
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
  ${CMAKE_BINARY_DIR}/version.h
  @ONLY
)

# Print information (like the original batch file)
message(STATUS "BUILD_ENV_NAME              : ${BUILD_ENV_NAME}")
message(STATUS "")
message(STATUS "GIT_REMOTE_ORIGIN_URL       : ${GIT_REMOTE_ORIGIN_URL}")
message(STATUS "GIT_COMMIT_HASH             : ${GIT_COMMIT_HASH}")
message(STATUS "GIT_SHORT_COMMIT_HASH       : ${GIT_SHORT_COMMIT_HASH}")
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
message(STATUS "version.h has been generated.")
