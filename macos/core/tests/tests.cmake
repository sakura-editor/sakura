# Mac 版核心库的单元测试：沿用原版 src/test/cpp/tests1 的测试代码。
# test-design_template.cpp 依赖 Windows 的模块句柄，不在 Mac 上运行。
# test-cfileext.cpp 依赖 env 的 CDocTypeManager，第 20 步加入。
# test-cconvert.cpp、test-cdecode.cpp 链接时需要 charset（basis/CEol → CCodeFactory），第 17 步加入。
# 只列出对应模块已经移植完成的测试文件。后续步骤每移植一个模块，就把相应的测试加进来。
set(SAKURA_CORE_TEST_SOURCES
  src/test/cpp/tests1/test-clayoutint.cpp
  src/test/cpp/tests1/test-cmemory.cpp
  src/test/cpp/tests1/test-cnative.cpp
  src/test/cpp/tests1/test-crunningtimer.cpp
  src/test/cpp/tests1/test-cwordparse.cpp
  src/test/cpp/tests1/test-editinfo.cpp
  src/test/cpp/tests1/test-format.cpp
  src/test/cpp/tests1/test-int2dec.cpp
  src/test/cpp/tests1/test-is_mailaddress.cpp
  src/test/cpp/tests1/test-statictype.cpp
  src/test/cpp/tests1/test-string_ex.cpp
)
list(TRANSFORM SAKURA_CORE_TEST_SOURCES PREPEND "${SAKURA_ROOT}/")
# Mac 版专用的测试（Win32 替代函数等）
list(APPEND SAKURA_CORE_TEST_SOURCES
  "${CMAKE_CURRENT_LIST_DIR}/test-mac-io.cpp"
)
