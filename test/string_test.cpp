// Date:   Wed Mar 19 05:15:51 PM 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "evo/string"
#include "gtest/gtest.h"
#include <cstring>
#include <string>

template <typename... Args>
static void cmp_std(Args&&... args) {
  evo::string s(std::forward<Args>(args)...);
  std::string ss(std::forward<Args>(args)...);
  ASSERT_TRUE(std::strcmp(s.c_str(), ss.c_str()) == 0);
}

TEST(StringTest, BasicTest) {
  cmp_std("hello");
  cmp_std("this is a really long string, this is a really long string");
  cmp_std(10, 'm');
  cmp_std(25, 'm');
}
