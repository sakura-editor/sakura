# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindNuGetGTest
-----------

Find the GoogleTest from NuGet repository.

Imported Targets
^^^^^^^^^^^^^^^^

This module defines any :ref:`Imported Targets <Imported Targets>`.

Result Variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project
(see :ref:`Standard Variable Names <CMake Developer Standard Variable Names>`):

``NUGET_GTEST_FOUND``
  System has the NuGet GoogleTest package.

Commands
^^^^^^^^

This module defines the following commands.

  NuGet_GTest_target_link_libraries(_target)

#]=======================================================================]

cmake_minimum_required(VERSION 3.12)

function(NuGet_GTest_target_link_libraries _target)
  # add preprocessor symbol definitions required for GoogleTest in MSVC
  target_compile_definitions(${_target} PRIVATE _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)

  # add NuGet.targets as a library
  target_link_libraries(${_target} PRIVATE ${NuGet_GTest_Root}/build/${NuGet_GTest_Framework}/${NuGet_GTest_Id}.targets)
endfunction()

# the NuGet Command Line Interface required.
find_package(NuGet REQUIRED)

# define variables for NuGet package of GoogleTest.
set(NuGet_GTest_Name		Microsoft.googletest)
set(NuGet_GTest_Id			Microsoft.googletest.v140.windesktop.msvcstl.static.rt-static)
set(NuGet_GTest_Version		1.8.0)
set(NuGet_GTest_Framework	native)
set(NuGet_GTest_Root		${NUGET_PACKAGES_DIR}/${NuGet_GTest_Id}.${NuGet_GTest_Version})

# find a NuGet.targets in the standard native NuGet package structure.
find_path(GTEST_TARGET ${NuGet_GTest_Id}.targets
              HINTS ${NuGet_GTest_Root}
      PATH_SUFFIXES /build/${NuGet_GTest_Framework})

find_package(PackageHandleStandardArgs QUIET)
find_package_handle_standard_args(GTest
                                  REQUIRED_VARS	GTEST_TARGET)
