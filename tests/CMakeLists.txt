cmake_minimum_required(VERSION 3.12)

enable_language(CXX)

if (BUILD_GTEST)
  # make GoogleTest: configure, build and install.
  function(install_gtest gtest_config)
    set(gtest_build_dir ${BUILD_BASE_DIR}${PLATFORM}/${gtest_config}/tests/googletest)
    file(MAKE_DIRECTORY ${gtest_build_dir})
    set(gtest_lib_dir "${GOOGLETEST_INSTALL_DIR}lib\\")
    if (CMAKE_INSTALL_LIBDIR)
      set(gtest_lib_dir "${GOOGLETEST_INSTALL_DIR}${CMAKE_INSTALL_LIBDIR}\\")
    endif (CMAKE_INSTALL_LIBDIR)
    set(gtest_lib_surfix "")
    if (gtest_config STREQUAL "Debug")
      set(gtest_lib_surfix "d")
    endif (gtest_config STREQUAL "Debug")
    if (NOT (EXISTS ${gtest_lib_dir}${CMAKE_STATIC_LIBRARY_PREFIX}gtest${gtest_lib_surfix}${CMAKE_STATIC_LIBRARY_SUFFIX} AND EXISTS ${gtest_lib_dir}${CMAKE_STATIC_LIBRARY_PREFIX}gtest_main${gtest_lib_surfix}${CMAKE_STATIC_LIBRARY_SUFFIX}))
      execute_process(
        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/googletest.build.cmd"
	        ${PLATFORM}
	        ${gtest_config}
	        ${GOOGLETEST_INSTALL_DIR}
        WORKING_DIRECTORY ${gtest_build_dir}
      )
    endif ()
  endfunction(install_gtest)
  
  install_gtest(Debug)
  install_gtest(Release)
  
  # re-find GoogleTest
  set(GTEST_ROOT ${GOOGLETEST_INSTALL_DIR})
endif (BUILD_GTEST)

# find GoogleTest
find_package(GTest REQUIRED)

# set Target Directories
add_subdirectory(unittests)
