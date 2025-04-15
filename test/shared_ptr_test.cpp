// Date:   Tue Apr 15 05:06:32 PM 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "evo/memory/shared_ptr"
#include <cassert>
#include <cstdio>

struct A {
  int n_;
  A(int n) noexcept:n_(n) {
    printf("A %d constructed\n", n_);
  }

  ~A() noexcept {
    printf("A %d destructed\n", n_);
  }
};

int main() {
  evo::shared_ptr<A> p1;
  assert(p1 == nullptr);

  evo::shared_ptr<A> p3;
  {
    auto p2 = evo::make_shared<A>(1);
    assert(p2 != nullptr);
    assert(p2->n_ == 1);

    p3 = p2;
    assert(p3 != nullptr);
    assert(p3->n_ == 1);
  }
  printf("p2 out of scope\n");

  assert(p3 != nullptr);
  assert(p3->n_ == 1);
}
