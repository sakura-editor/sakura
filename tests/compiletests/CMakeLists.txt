cmake_minimum_required(VERSION 3.12)

enable_language(CXX)

set( SAKURA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../sakura_core)
set( SAKURA_DEFINITIONS -D_DEBUG -D_UNICODE -DUNICODE -D_WIN32_WINNT=_WIN32_WINNT_WIN7)

# コンパイルテスト実行関数
function( compile_test TEST_TEMPLATE TEST_BODY TEST_DESCRIPTION TEST_NAME )
  message( STATUS "Checking ${TEST_NAME}" )

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${TEST_TEMPLATE}
    ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}.cpp
    NEWLINE_STYLE WIN32
    )

  try_compile(FAILED_TO_TEST ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}.cpp
    CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${SAKURA_SOURCE_DIR}"
    COMPILE_DEFINITIONS ${SAKURA_DEFINITIONS}
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    )

  if (NOT FAILED_TO_TEST)
    message( STATUS "Checking ${TEST_NAME} - done" )
  else()
    message( FATAL_ERROR "Checking ${TEST_NAME} - Failed." )
  endif (NOT FAILED_TO_TEST)
endfunction( compile_test )

# コンパイルテストの定義
compile_test( clayoutint_test.cpp.in "a = b;" "CLayoutInt に CLogicIntを代入することはできない" clayoutint_can_not_be_assigned_from_clogicint )
