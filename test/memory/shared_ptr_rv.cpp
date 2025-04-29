// Date:   Tue Apr 29 20:23:04 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03

// <memory>

// shared_ptr

// shared_ptr& operator=(shared_ptr&& r);

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

TEST(SharedPtrTest, SharedPtrRv) {
  {
    std::shared_ptr<A> pA(new A);
    A* ptrA = pA.get();
    {
      std::shared_ptr<A> pB(new A);
      pB = std::move(pA);
      ASSERT_TRUE(B::count == 1);
      ASSERT_TRUE(A::count == 1);
      ASSERT_TRUE(pB.use_count() == 1);
      ASSERT_TRUE(pA.use_count() == 0);
      ASSERT_TRUE(pA.get() == 0);
      ASSERT_TRUE(pB.get() == ptrA);
    }
    ASSERT_TRUE(pA.use_count() == 0);
    ASSERT_TRUE(B::count == 0);
    ASSERT_TRUE(A::count == 0);
  }
  ASSERT_TRUE(B::count == 0);
  ASSERT_TRUE(A::count == 0);
  {
    std::shared_ptr<A> pA;
    A* ptrA = pA.get();
    {
      std::shared_ptr<A> pB(new A);
      pB = std::move(pA);
      ASSERT_TRUE(B::count == 0);
      ASSERT_TRUE(A::count == 0);
      ASSERT_TRUE(pB.use_count() == 0);
      ASSERT_TRUE(pA.use_count() == 0);
      ASSERT_TRUE(pA.get() == 0);
      ASSERT_TRUE(pB.get() == ptrA);
    }
    ASSERT_TRUE(pA.use_count() == 0);
    ASSERT_TRUE(B::count == 0);
    ASSERT_TRUE(A::count == 0);
  }
  ASSERT_TRUE(B::count == 0);
  ASSERT_TRUE(A::count == 0);
  {
    std::shared_ptr<A> pA(new A);
    A* ptrA = pA.get();
    {
      std::shared_ptr<A> pB;
      pB = std::move(pA);
      ASSERT_TRUE(B::count == 1);
      ASSERT_TRUE(A::count == 1);
      ASSERT_TRUE(pB.use_count() == 1);
      ASSERT_TRUE(pA.use_count() == 0);
      ASSERT_TRUE(pA.get() == 0);
      ASSERT_TRUE(pB.get() == ptrA);
    }
    ASSERT_TRUE(pA.use_count() == 0);
    ASSERT_TRUE(B::count == 0);
    ASSERT_TRUE(A::count == 0);
  }
  ASSERT_TRUE(B::count == 0);
  ASSERT_TRUE(A::count == 0);
  {
    std::shared_ptr<A> pA;
    A* ptrA = pA.get();
    {
      std::shared_ptr<A> pB;
      pB = std::move(pA);
      ASSERT_TRUE(B::count == 0);
      ASSERT_TRUE(A::count == 0);
      ASSERT_TRUE(pB.use_count() == 0);
      ASSERT_TRUE(pA.use_count() == 0);
      ASSERT_TRUE(pA.get() == 0);
      ASSERT_TRUE(pB.get() == ptrA);
    }
    ASSERT_TRUE(pA.use_count() == 0);
    ASSERT_TRUE(B::count == 0);
    ASSERT_TRUE(A::count == 0);
  }
  ASSERT_TRUE(B::count == 0);
  ASSERT_TRUE(A::count == 0);
}
