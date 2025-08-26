# CMake generated Testfile for 
# Source directory: /home/runner/work/sakura/sakura/tests
# Build directory: /home/runner/work/sakura/sakura/tests/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(unitTest1 "/home/runner/work/sakura/sakura/tests/build/bin/tests1")
set_tests_properties(unitTest1 PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/sakura/sakura/tests/CMakeLists.txt;47;add_test;/home/runner/work/sakura/sakura/tests/CMakeLists.txt;0;")
subdirs("googletest")
subdirs("compiletests")
subdirs("unittests")
