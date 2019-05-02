# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindNuGet
-----------

Find NuGet command line interface.

Imported Targets
^^^^^^^^^^^^^^^^

This module defines any :ref:`Imported Targets <Imported Targets>`.

Result Variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project
(see :ref:`Standard Variable Names <CMake Developer Standard Variable Names>`):

``NUGET_FOUND``
  System has the NuGet command line interface.
``NUGET_EXECUTABLE``
  Path to the NuGet command line interface.
``NUGET_VERSION``
  File version of the NuGet command line interface.
``NUGET_PACKAGES_DIR``
  Path to a NuGet package directory.

Hints
^^^^^

``NUGET_ROOT``
  Define the root directory of a NuGet command line interface installation.
``NUGET_PACKAGES_DIR``
  Define the root directory of a NuGet package directory.

Commands
^^^^^^^^

This module defines the following commands in your project

  NuGet_restore_package(_packages_config)

#]=======================================================================]

# restore NuGet packages directly in configure phase.
function(NuGet_restore_package _packages_config)
  execute_process(
    COMMAND
	  ${NUGET_EXECUTABLE} restore
	    -PackagesDirectory ${NUGET_PACKAGES_DIR}
		${_packages_config}
  )
endfunction()

if(NOT NUGET_PACKAGES_DIR)
  set(NUGET_PACKAGES_DIR ${CMAKE_LIST_DIR}/packages)
endif()
mark_as_advanced(NUGET_PACKAGES_DIR)

# find NuGet Command Line Interface.
find_program(NUGET_EXECUTABLE nuget
                        HINTS ${NUGET_ROOT})

if(NUGET_EXECUTABLE)
  ### NUGET_VERSION
  set(NUGET_VERSION "Unknown")
  # check if powershell interpreter is available.
  find_program(POWERSHELL_EXECUTABLE powershell)
  if(POWERSHELL_EXECUTABLE)
    execute_process(
      COMMAND
        powershell -ExecutionPolicy RemoteSigned -Command "${NUGET_EXECUTABLE} help | select -First 1"
        OUTPUT_VARIABLE
          NUGET_VERSION_OUTPUT_VARIABLE
        RESULT_VARIABLE
          NUGET_VERSION_RESULT_VARIABLE
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  endif()
  if(NUGET_VERSION_RESULT_VARIABLE)
    string(REGEX REPLACE "NuGet Version: (.+)" "\\1" NUGET_VERSION ${NUGET_VERSION_OUTPUT_VARIABLE})
  endif()
endif()

find_package(PackageHandleStandardArgs QUIET)
find_package_handle_standard_args(NuGet
                                  REQUIRED_VARS	NUGET_EXECUTABLE
                                  VERSION_VAR	NUGET_VERSION)

mark_as_advanced(NUGET_EXECUTABLE)
