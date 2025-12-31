# Helper script to update a git submodule with a cross-target lock.
#
# Parameters (passed via -D):
#   GIT_EXECUTABLE  : path to git.exe
#   REPO_ROOT       : repository root directory
#   SUBMODULE_PATH  : submodule path (e.g. externals/miniz-cpp)
#   LOCK_PATH       : lock file path (optional)

if(NOT DEFINED GIT_EXECUTABLE)
  message(FATAL_ERROR "GIT_EXECUTABLE is required")
endif()

if(NOT DEFINED REPO_ROOT)
  message(FATAL_ERROR "REPO_ROOT is required")
endif()

if(NOT DEFINED SUBMODULE_PATH)
  message(FATAL_ERROR "SUBMODULE_PATH is required")
endif()

if(NOT DEFINED LOCK_PATH)
  set(LOCK_PATH "${REPO_ROOT}/.git/cmake-submodule-update.lock")
endif()

# Serialize concurrent submodule updates triggered by parallel builds.
file(LOCK "${LOCK_PATH}" TIMEOUT 600)

execute_process(
  COMMAND "${GIT_EXECUTABLE}" submodule update --init --recursive --depth 1 --recommend-shallow "${SUBMODULE_PATH}"
  WORKING_DIRECTORY "${REPO_ROOT}"
  RESULT_VARIABLE result
  OUTPUT_VARIABLE stdout
  ERROR_VARIABLE stderr
)

if(NOT result EQUAL 0)
  set(hint "")
  if(EXISTS "${REPO_ROOT}/.git/config.lock")
    set(hint "\nHint: A lock file exists: ${REPO_ROOT}/.git/config.lock\nIf no git process is running, delete it and retry.")
  endif()
  message(FATAL_ERROR "FAILED: [code=${result}] ${REPO_ROOT}/${SUBMODULE_PATH}/.git\n${stdout}\n${stderr}${hint}")
endif()
