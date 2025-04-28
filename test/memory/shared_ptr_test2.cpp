// Date:   Mon Apr 28 21:15:17 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include <gtest/gtest.h>
#include "evo/memory/shared_ptr"

struct B {
  static int count;

  B() { ++count; }
  B(const B&) { ++count; }
  virtual ~B() { --count; }
};

int B::count = 0;

struct A : public B {
  static int count;

  A() { ++count; }
  A(const A& other) : B(other) { ++count; }
  ~A() { --count; }
};

int A::count = 0;

TEST(SharedPtrTest, AssignmentAndCounts) {
  {
    const evo::shared_ptr<A> pA(new A);
    A* ptrA = pA.get();
    {
      evo::shared_ptr<A> pB(new A);
      pB = pA;
      EXPECT_EQ(B::count, 1);
      EXPECT_EQ(A::count, 1);
      EXPECT_EQ(pB.use_count(), 2);
      EXPECT_EQ(pA.use_count(), 2);
      EXPECT_EQ(pA.get(), pB.get());
      EXPECT_EQ(pB.get(), ptrA);
    }
    EXPECT_EQ(pA.use_count(), 1);
    EXPECT_EQ(B::count, 1);
    EXPECT_EQ(A::count, 1);
  }
  EXPECT_EQ(B::count, 0);
  EXPECT_EQ(A::count, 0);

  {
    const evo::shared_ptr<A> pA;
    A* ptrA = pA.get();
    {
      evo::shared_ptr<A> pB(new A);
      pB = pA;
      EXPECT_EQ(B::count, 0);
      EXPECT_EQ(A::count, 0);
      EXPECT_EQ(pB.use_count(), 0);
      EXPECT_EQ(pA.use_count(), 0);
      EXPECT_EQ(pA.get(), pB.get());
      EXPECT_EQ(pB.get(), ptrA);
    }
    EXPECT_EQ(pA.use_count(), 0);
    EXPECT_EQ(B::count, 0);
    EXPECT_EQ(A::count, 0);
  }
  EXPECT_EQ(B::count, 0);
  EXPECT_EQ(A::count, 0);

  {
    const evo::shared_ptr<A> pA(new A);
    A* ptrA = pA.get();
    {
      evo::shared_ptr<A> pB;
      pB = pA;
      EXPECT_EQ(B::count, 1);
      EXPECT_EQ(A::count, 1);
      EXPECT_EQ(pB.use_count(), 2);
      EXPECT_EQ(pA.use_count(), 2);
      EXPECT_EQ(pA.get(), pB.get());
      EXPECT_EQ(pB.get(), ptrA);
    }
    EXPECT_EQ(pA.use_count(), 1);
    EXPECT_EQ(B::count, 1);
    EXPECT_EQ(A::count, 1);
  }
  EXPECT_EQ(B::count, 0);
  EXPECT_EQ(A::count, 0);

  {
    const evo::shared_ptr<A> pA;
    A* ptrA = pA.get();
    {
      evo::shared_ptr<A> pB;
      pB = pA;
      EXPECT_EQ(B::count, 0);
      EXPECT_EQ(A::count, 0);
      EXPECT_EQ(pB.use_count(), 0);
      EXPECT_EQ(pA.use_count(), 0);
      EXPECT_EQ(pA.get(), pB.get());
      EXPECT_EQ(pB.get(), ptrA);
    }
    EXPECT_EQ(pA.use_count(), 0);
    EXPECT_EQ(B::count, 0);
    EXPECT_EQ(A::count, 0);
  }
  EXPECT_EQ(B::count, 0);
  EXPECT_EQ(A::count, 0);
}
