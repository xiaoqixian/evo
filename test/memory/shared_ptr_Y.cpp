// Date:   Tue Apr 29 02:31:28 PM 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <memory>

// shared_ptr

// template<class Y> shared_ptr& operator=(const shared_ptr<Y>& r);

#include <gtest/gtest.h>
#include "evo/memory/shared_ptr"

struct B
{
  static int count;

  B() {++count;}
  B(const B&) {++count;}
  virtual ~B() {--count;}
};

int B::count = 0;

struct A
: public B
{
  static int count;

  A() {++count;}
  A(const A& other) : B(other) {++count;}
  ~A() {--count;}
};

int A::count = 0;

TEST(SharedPtrTest, SharedPtrY) {
  // {
  //   const evo::shared_ptr<A> pA(new A);
  //   A* ptrA = pA.get();
  //   {
  //     evo::shared_ptr<B> pB(new B);
  //     pB = pA;
  //     ASSERT_TRUE(B::count == 1);
  //     ASSERT_TRUE(A::count == 1);
  //     ASSERT_TRUE(pB.use_count() == 2);
  //     ASSERT_TRUE(pA.use_count() == 2);
  //     ASSERT_TRUE(pA.get() == pB.get());
  //     ASSERT_TRUE(pB.get() == ptrA);
  //   }
  //   ASSERT_TRUE(pA.use_count() == 1);
  //   ASSERT_TRUE(B::count == 1);
  //   ASSERT_TRUE(A::count == 1);
  // }
  // ASSERT_TRUE(B::count == 0);
  // ASSERT_TRUE(A::count == 0);
  // {
  //   const evo::shared_ptr<A> pA;
  //   A* ptrA = pA.get();
  //   {
  //     evo::shared_ptr<B> pB(new B);
  //     pB = pA;
  //     ASSERT_TRUE(B::count == 0);
  //     ASSERT_TRUE(A::count == 0);
  //     ASSERT_TRUE(pB.use_count() == 0);
  //     ASSERT_TRUE(pA.use_count() == 0);
  //     ASSERT_TRUE(pA.get() == pB.get());
  //     ASSERT_TRUE(pB.get() == ptrA);
  //   }
  //   ASSERT_TRUE(pA.use_count() == 0);
  //   ASSERT_TRUE(B::count == 0);
  //   ASSERT_TRUE(A::count == 0);
  // }
  // ASSERT_TRUE(B::count == 0);
  // ASSERT_TRUE(A::count == 0);
  // {
  //   const evo::shared_ptr<A> pA(new A);
  //   A* ptrA = pA.get();
  //   {
  //     evo::shared_ptr<B> pB;
  //     pB = pA;
  //     ASSERT_TRUE(B::count == 1);
  //     ASSERT_TRUE(A::count == 1);
  //     ASSERT_TRUE(pB.use_count() == 2);
  //     ASSERT_TRUE(pA.use_count() == 2);
  //     ASSERT_TRUE(pA.get() == pB.get());
  //     ASSERT_TRUE(pB.get() == ptrA);
  //   }
  //   ASSERT_TRUE(pA.use_count() == 1);
  //   ASSERT_TRUE(B::count == 1);
  //   ASSERT_TRUE(A::count == 1);
  // }
  // ASSERT_TRUE(B::count == 0);
  // ASSERT_TRUE(A::count == 0);
  // {
  //   const evo::shared_ptr<A> pA;
  //   A* ptrA = pA.get();
  //   {
  //     evo::shared_ptr<B> pB;
  //     pB = pA;
  //     ASSERT_TRUE(B::count == 0);
  //     ASSERT_TRUE(A::count == 0);
  //     ASSERT_TRUE(pB.use_count() == 0);
  //     ASSERT_TRUE(pA.use_count() == 0);
  //     ASSERT_TRUE(pA.get() == pB.get());
  //     ASSERT_TRUE(pB.get() == ptrA);
  //   }
  //   ASSERT_TRUE(pA.use_count() == 0);
  //   ASSERT_TRUE(B::count == 0);
  //   ASSERT_TRUE(A::count == 0);
  // }
  // ASSERT_TRUE(B::count == 0);
  // ASSERT_TRUE(A::count == 0);

  {
    evo::shared_ptr<A[]> p1(new A[8]);
    A* ptr = p1.get();
    ASSERT_TRUE(A::count == 8);
    {
      evo::shared_ptr<const A[]> p2;
      p2 = p1;
      ASSERT_TRUE(A::count == 8);
      ASSERT_TRUE(p2.use_count() == 2);
      ASSERT_TRUE(p1.use_count() == 2);
      ASSERT_TRUE(p1.get() == p2.get());
      ASSERT_TRUE(p2.get() == ptr);
    }
    ASSERT_TRUE(p1.use_count() == 1);
    ASSERT_TRUE(A::count == 8);
  }
  ASSERT_TRUE(A::count == 0);
}
