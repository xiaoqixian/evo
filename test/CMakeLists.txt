# Date:   Mon Mar 17 04:25:08 PM 2025
# Mail:   lunar_ubuntu@qq.com
# Author: https://github.com/xiaoqixian

include(GoogleTest)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE EVO_TEST_SRC "${PROJECT_SOURCE_DIR}/test/*test.cpp")

foreach (test_src ${EVO_TEST_SRC})
  # extract the test SOURCE filename, remove the suffix to create a test name
  get_filename_component(test_filename ${test_src} NAME)
  string(REPLACE ".cpp" "" test_name ${test_filename})
  
  message(STATUS "Add test ${test_name}")
  add_executable(${test_name} EXCLUDE_FROM_ALL ${test_src})

  target_link_libraries(${test_name} evo gtest)
endforeach ()
