cmake_minimum_required(VERSION 3.12)

enable_language(CXX)

message( STATUS "Checking clayoutint_can_not_be_assigned_from_clogicint" )

try_compile(FAILED_TO_TEST001 ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/clayoutint_can_not_be_assigned_from_clogicint.cpp
  CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${CMAKE_CURRENT_SOURCE_DIR}/../../sakura_core"
  COMPILE_DEFINITIONS -D_DEBUG -D_UNICODE -DUNICODE -D_WIN32_WINNT=_WIN32_WINNT_WIN7
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  )

if (NOT FAILED_TO_TEST001)
  message( STATUS "Checking clayoutint_can_not_be_assigned_from_clogicint - done" )
else()
  message( FATAL_ERROR "Checking clayoutint_can_not_be_assigned_from_clogicint - Failed." )
endif (NOT FAILED_TO_TEST001)
