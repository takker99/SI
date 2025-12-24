# CMake generated Testfile for 
# Source directory: /home/runner/work/SI/SI/test
# Build directory: /home/runner/work/SI/SI/_codeql_build_dir/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
include("/home/runner/work/SI/SI/_codeql_build_dir/test/SI_base_unit_tests_include-b12d07c.cmake")
include("/home/runner/work/SI/SI/_codeql_build_dir/test/SI_detail_tests_include-b12d07c.cmake")
include("/home/runner/work/SI/SI/_codeql_build_dir/test/SI_derived_unit_tests_include-b12d07c.cmake")
add_test([=[SI_detail_tests]=] "/home/runner/work/SI/SI/_codeql_build_dir/test/SI_detail_tests")
set_tests_properties([=[SI_detail_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/SI/SI/test/CMakeLists.txt;151;add_test;/home/runner/work/SI/SI/test/CMakeLists.txt;0;")
add_test([=[SI_base_unit_tests]=] "/home/runner/work/SI/SI/_codeql_build_dir/test/SI_base_unit_tests")
set_tests_properties([=[SI_base_unit_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/SI/SI/test/CMakeLists.txt;152;add_test;/home/runner/work/SI/SI/test/CMakeLists.txt;0;")
add_test([=[SI_derived_unit_tests]=] "/home/runner/work/SI/SI/_codeql_build_dir/test/SI_derived_unit_tests")
set_tests_properties([=[SI_derived_unit_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/SI/SI/test/CMakeLists.txt;153;add_test;/home/runner/work/SI/SI/test/CMakeLists.txt;0;")
