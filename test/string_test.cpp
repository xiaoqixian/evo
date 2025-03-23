// Date:   Wed Mar 19 05:15:51 PM 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "evo/string"
#include <cstdio>

int main() {
  evo::string s("hello");
  printf("%s\n", s.c_str());
  evo::string s2("this is a really long string, this is a really long string");
  printf("%s\n", s2.c_str());
}
