// Date:   Tue Apr 15 05:06:32 PM 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "evo/memory/shared_ptr"
#include <gtest/gtest.h>

struct A {
    int value;
    A(int v) : value(v) {}
};

TEST(SharedPtrTest, BasicCreate) {
    evo::shared_ptr<A> p = evo::make_shared<A>(42);
    EXPECT_TRUE(p != nullptr);
    EXPECT_EQ(p->value, 42);
}

TEST(SharedPtrTest, CopySharedPtr) {
    auto p1 = evo::make_shared<A>(100);
    auto p2 = p1;
    EXPECT_EQ(p1.use_count(), 2);
    EXPECT_EQ(p2.use_count(), 2);
    EXPECT_EQ(p1->value, p2->value);
}

TEST(SharedPtrTest, ResetSharedPtr) {
    auto p = evo::make_shared<A>(10);
    EXPECT_TRUE(p != nullptr);
    p.reset();
    EXPECT_EQ(p, nullptr);
}

TEST(SharedPtrTest, SwapSharedPtr) {
    auto p1 = evo::make_shared<A>(1);
    auto p2 = evo::make_shared<A>(2);
    evo::swap(p1, p2);
    EXPECT_EQ(p1->value, 2);
    EXPECT_EQ(p2->value, 1);
}

TEST(SharedPtrTest, UniqueOwnership) {
    auto p1 = evo::make_shared<A>(5);
    EXPECT_EQ(p1.use_count(), 1);
    {
        auto p2 = p1;
        EXPECT_EQ(p1.use_count(), 2);
        EXPECT_EQ(p2.use_count(), 2);
    }
    EXPECT_EQ(p1.use_count(), 1);
}

TEST(SharedPtrTest, GetAndDereference) {
    auto p = evo::make_shared<A>(7);
    A* raw = p.get();
    EXPECT_EQ(raw->value, 7);
    EXPECT_EQ((*p).value, 7);
}

TEST(SharedPtrTest, WeakPtrLock) {
    evo::shared_ptr<A> sp = evo::make_shared<A>(88);
    evo::weak_ptr<A> wp = sp;

    auto locked = wp.lock();
    EXPECT_TRUE(locked != nullptr);
    EXPECT_EQ(locked->value, 88);
}

TEST(SharedPtrTest, WeakPtrLockExpired) {
    evo::weak_ptr<A> wp;
    {
        auto sp = evo::make_shared<A>(99);
        wp = sp;
        EXPECT_FALSE(wp.expired());
    }
    EXPECT_TRUE(wp.expired());

    auto locked = wp.lock();
    EXPECT_EQ(locked, nullptr);
}

TEST(SharedPtrTest, WeakPtrExpired) {
    auto sp = evo::make_shared<A>(123);
    evo::weak_ptr<A> wp = sp;

    EXPECT_FALSE(wp.expired());
    sp.reset();
    EXPECT_TRUE(wp.expired());
}

TEST(SharedPtrTest, MoveSharedPtr) {
    auto p1 = evo::make_shared<A>(555);
    auto p2 = std::move(p1);
    EXPECT_EQ(p1, nullptr);
    EXPECT_TRUE(p2 != nullptr);
    EXPECT_EQ(p2->value, 555);
}

TEST(SharedPtrTest, MoveWeakPtr) {
    auto sp = evo::make_shared<A>(333);
    evo::weak_ptr<A> wp1 = sp;
    evo::weak_ptr<A> wp2 = std::move(wp1);

    EXPECT_TRUE(wp1.expired());
    EXPECT_FALSE(wp2.expired());
}

TEST(SharedPtrTest, UseCountCheck) {
    auto sp = evo::make_shared<A>(1);
    evo::weak_ptr<A> wp = sp;

    EXPECT_EQ(sp.use_count(), 1);
    {
        auto sp2 = wp.lock();
        EXPECT_EQ(sp.use_count(), 2);
    }
    EXPECT_EQ(sp.use_count(), 1);
}

TEST(SharedPtrTest, WeakPtrReset) {
    auto sp = evo::make_shared<A>(200);
    evo::weak_ptr<A> wp = sp;

    EXPECT_FALSE(wp.expired());
    wp.reset();
    EXPECT_TRUE(wp.expired());
}
