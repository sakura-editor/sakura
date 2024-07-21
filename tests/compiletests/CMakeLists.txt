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
compile_test( clayoutint_test.cpp.in "a += b;" "CLayoutInt に CLogicIntを加算代入することはできない" clayoutint_can_not_be_additive_assigned_from_clogicint )
compile_test( clayoutint_test.cpp.in "a -= b;" "CLayoutInt に CLogicIntを減算代入することはできない" clayoutint_can_not_be_subtractive_assigned_from_clogicint )
compile_test( clayoutint_test.cpp.in "a = a + b;" "CLayoutInt と CLogicIntを加算することはできない" clayoutint_can_not_be_added_by_clogicint )
compile_test( clayoutint_test.cpp.in "a = a - b;" "CLayoutInt から CLogicIntを減算することはできない" clayoutint_can_not_be_subtracted_by_clogicint )
compile_test( clayoutint_test.cpp.in "a = b;" "CLayoutInt に CLogicIntを代入することはできない" clayoutint_can_not_be_assigned_from_clogicint )
compile_test( clayoutint_test.cpp.in "a < b;" "CLayoutInt と CLogicIntを比較することはできない" clayoutint_can_not_be_compare_with_clogicint_less_than )
compile_test( clayoutint_test.cpp.in "a <= b;" "CLayoutInt と CLogicIntを比較することはできない" clayoutint_can_not_be_compare_with_clogicint_less_or_equal )
compile_test( clayoutint_test.cpp.in "a > b;" "CLayoutInt と CLogicIntを比較することはできない" clayoutint_can_not_be_compare_with_clogicint_greater_than )
compile_test( clayoutint_test.cpp.in "a >= b;" "CLayoutInt と CLogicIntを比較することはできない" clayoutint_can_not_be_compare_with_clogicint_greater_or_equal )
compile_test( clayoutint_test.cpp.in "a == b;" "CLayoutInt と CLogicIntを比較することはできない" clayoutint_can_not_be_compare_with_clogicint_equal )
compile_test( clayoutint_test.cpp.in "a != b;" "CLayoutInt と CLogicIntを比較することはできない" clayoutint_can_not_be_compare_with_clogicint_not_equal )
compile_test( clayoutint_test.cpp.in "int c = a;" "CLayoutInt は キャストなしでint型に代入することはできない" clayoutint_can_not_be_assigned_to_int )
