// Date:   Tue Apr 29 20:27:11 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17

// <memory>

// shared_ptr

// template<class T> shared_ptr<T> make_shared(size_t N); // T is U[]
//
// template<class T>
// shared_ptr<T> make_shared(size_t N, const remove_extent_t<T>& u); // T is U[]

// Ignore error about requesting a large alignment not being ABI compatible with older AIX systems.

#include "evo/memory/shared_ptr"
#include <gtest/gtest.h>

struct DestroyInReverseOrder {
  static void reset() { global_count_ = 0; }
  static int alive() { return global_count_; }

  DestroyInReverseOrder()
    : DestroyInReverseOrder(&global_count_)
  { }

  constexpr DestroyInReverseOrder(int* count)
    : count_(count), value_(*count)
  { ++*count_; }

  constexpr DestroyInReverseOrder(DestroyInReverseOrder const& other)
    : count_(other.count_), value_(*other.count_)
  { ++*count_; }

  constexpr int value() const { return value_; }

  // Ensure that we destroy these objects in the reverse order as they were created.
  constexpr ~DestroyInReverseOrder() {
    --*count_;
    assert(*count_ == value_);
  }

private:
  int* count_;
  int value_;
  static int global_count_;
};

int DestroyInReverseOrder::global_count_ = 0;

struct NonMovable {
  NonMovable() = default;
  NonMovable(NonMovable&&) = delete;
};

struct CountCopies {
  static void reset() { global_count_ = 0; }
  static int copies() { return global_count_; }

  constexpr CountCopies() : copies_(&global_count_) { }
  constexpr CountCopies(int* counter) : copies_(counter) { }
  constexpr CountCopies(CountCopies const& other) : copies_(other.copies_) { ++*copies_; }

private:
  int* copies_;
  static int global_count_;
};

int CountCopies::global_count_ = 0;

struct alignas(alignof(std::max_align_t) * 2) OverAligned { };

struct MaxAligned {
  std::max_align_t foo;
};

struct ThrowOnConstruction {
  struct exception : std::exception { };

  ThrowOnConstruction() { on_construct(); }
  ThrowOnConstruction(ThrowOnConstruction const&) { on_construct(); }

  static void reset() { throw_after_ = -1; }
  static void throw_after(int n) { throw_after_ = n; }

private:
  static int throw_after_;
  void on_construct() {
    if (throw_after_ == 0)
      throw exception{};

    if (throw_after_ != -1)
      --throw_after_;
  }
};

int ThrowOnConstruction::throw_after_ = -1;

template <class T, class ...Args>
concept CanMakeShared = requires(Args&& ...args) {
  { evo::make_shared<T>(std::forward<Args>(args)...) } -> std::same_as<evo::shared_ptr<T>>;
};

TEST(SharedPtrTest, SharedPtrUnboundArray) {
  // Check behavior for a zero-sized array
  {
    // Without passing an initial value
    {
      using Array = int[];
      evo::shared_ptr<Array> ptr = evo::make_shared<Array>(0);
      ASSERT_TRUE(ptr != nullptr);
    }

    // Passing an initial value
    {
      using Array = int[];
      evo::shared_ptr<Array> ptr = evo::make_shared<Array>(0, 42);
      ASSERT_TRUE(ptr != nullptr);
    }
  }

  // Check behavior for a 1-sized array
  {
    // Without passing an initial value
    {
      using Array = int[];
      evo::shared_ptr<Array> ptr = evo::make_shared<Array>(1);
      ASSERT_TRUE(ptr != nullptr);
      ASSERT_TRUE(ptr[0] == 0);
    }

    // Passing an initial value
    {
      using Array = int[];
      evo::shared_ptr<Array> ptr = evo::make_shared<Array>(1, 42);
      ASSERT_TRUE(ptr != nullptr);
      ASSERT_TRUE(ptr[0] == 42);
    }
  }

  // Make sure we initialize elements correctly
  {
    // Without passing an initial value
    {
      using Array = int[];
      evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8);
      for (unsigned i = 0; i < 8; ++i) {
        ASSERT_TRUE(ptr[i] == 0);
      }
    }
    {
      using Array = int[][3];
      evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8);
      for (unsigned i = 0; i < 8; ++i) {
        ASSERT_TRUE(ptr[i][0] == 0);
        ASSERT_TRUE(ptr[i][1] == 0);
        ASSERT_TRUE(ptr[i][2] == 0);
      }
    }
    {
      using Array = int[][3][2];
      evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8);
      for (unsigned i = 0; i < 8; ++i) {
        ASSERT_TRUE(ptr[i][0][0] == 0);
        ASSERT_TRUE(ptr[i][0][1] == 0);
        ASSERT_TRUE(ptr[i][1][0] == 0);
        ASSERT_TRUE(ptr[i][1][1] == 0);
        ASSERT_TRUE(ptr[i][2][0] == 0);
        ASSERT_TRUE(ptr[i][2][1] == 0);
      }
    }

    // Passing an initial value
    {
      using Array = int[];
      int init = 42;
      evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8, init);
      for (unsigned i = 0; i < 8; ++i) {
        ASSERT_TRUE(ptr[i] == init);
      }
    }
    {
      using Array = int[][3];
      int init[3] = {42, 43, 44};
      evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8, init);
      for (unsigned i = 0; i < 8; ++i) {
        ASSERT_TRUE(ptr[i][0] == 42);
        ASSERT_TRUE(ptr[i][1] == 43);
        ASSERT_TRUE(ptr[i][2] == 44);
      }
    }
    {
      using Array = int[][3][2];
      int init[3][2] = {{31, 32}, {41, 42}, {51, 52}};
      evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8, init);
      for (unsigned i = 0; i < 8; ++i) {
        ASSERT_TRUE(ptr[i][0][0] == 31);
        ASSERT_TRUE(ptr[i][0][1] == 32);
        ASSERT_TRUE(ptr[i][1][0] == 41);
        ASSERT_TRUE(ptr[i][1][1] == 42);
        ASSERT_TRUE(ptr[i][2][0] == 51);
        ASSERT_TRUE(ptr[i][2][1] == 52);
      }
    }
  }

  // Make sure array elements are destroyed in reverse order
  {
    // Without passing an initial value
    {
      using Array = DestroyInReverseOrder[];
      DestroyInReverseOrder::reset();
      {
        evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8);
        ASSERT_TRUE(DestroyInReverseOrder::alive() == 8);
      }
      ASSERT_TRUE(DestroyInReverseOrder::alive() == 0);
    }
    {
      using Array = DestroyInReverseOrder[][3];
      DestroyInReverseOrder::reset();
      {
        evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8);
        ASSERT_TRUE(DestroyInReverseOrder::alive() == 8 * 3);
      }
      ASSERT_TRUE(DestroyInReverseOrder::alive() == 0);
    }
    {
      using Array = DestroyInReverseOrder[][3][2];
      DestroyInReverseOrder::reset();
      {
        evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8);
        ASSERT_TRUE(DestroyInReverseOrder::alive() == 8 * 3 * 2);
      }
      ASSERT_TRUE(DestroyInReverseOrder::alive() == 0);
    }

    // Passing an initial value
    {
      using Array = DestroyInReverseOrder[];
      int count = 0;
      DestroyInReverseOrder init(&count);
      int init_count = 1;
      {
        evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8, init);
        ASSERT_TRUE(count == 8 + init_count);
      }
      ASSERT_TRUE(count == init_count);
    }
    {
      using Array = DestroyInReverseOrder[][3];
      int count = 0;
      DestroyInReverseOrder init[3] = {&count, &count, &count};
      int init_count = 3;
      {
        evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8, init);
        ASSERT_TRUE(count == 8 * 3 + init_count);
      }
      ASSERT_TRUE(count == init_count);
    }
    {
      using Array = DestroyInReverseOrder[][3][2];
      int count = 0;
      DestroyInReverseOrder init[3][2] = {{&count, &count}, {&count, &count}, {&count, &count}};
      int init_count = 3 * 2;
      {
        evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8, init);
        ASSERT_TRUE(count == 8 * 3 * 2 + init_count);
      }
      ASSERT_TRUE(count == init_count);
    }
  }

  // Count the number of copies being made
  {
    // Without passing an initial value
    {
      using Array = CountCopies[];
      CountCopies::reset();
      evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8);
      ASSERT_TRUE(CountCopies::copies() == 0);
    }
    {
      using Array = CountCopies[][3];
      CountCopies::reset();
      evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8);
      ASSERT_TRUE(CountCopies::copies() == 0);
    }
    {
      using Array = CountCopies[][3][2];
      CountCopies::reset();
      evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8);
      ASSERT_TRUE(CountCopies::copies() == 0);
    }

    // Passing an initial value
    {
      using Array = CountCopies[];
      int copies = 0;
      CountCopies init(&copies);
      evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8, init);
      ASSERT_TRUE(copies == 8);
    }
    {
      using Array = CountCopies[][3];
      int copies = 0;
      CountCopies init[3] = {&copies, &copies, &copies};
      evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8, init);
      ASSERT_TRUE(copies == 8 * 3);
    }
    {
      using Array = CountCopies[][3][2];
      int copies = 0;
      CountCopies init[3][2] = {{&copies, &copies}, {&copies, &copies}, {&copies, &copies}};
      evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8, init);
      ASSERT_TRUE(copies == 8 * 3 * 2);
    }
  }

  // Make sure array elements are aligned properly when the array contains an overaligned type.
  //
  // Here, we don't need to test both the with-initial-value and without-initial-value code paths,
  // since we're just checking the alignment and both are going to use the same code path unless
  // the implementation is completely crazy.
  {
    auto check_alignment = []<class T> {
      {
        using Array = T[];
        evo::shared_ptr ptr = evo::make_shared<Array>(8);
        for (int i = 0; i < 8; ++i) {
          T* p = std::addressof(ptr[i]);
          ASSERT_TRUE(reinterpret_cast<std::uintptr_t>(p) % alignof(T) == 0);
        }
      }
      {
        using Array = T[][3];
        evo::shared_ptr ptr = evo::make_shared<Array>(8);
        for (int i = 0; i < 8; ++i) {
          for (int j = 0; j < 3; ++j) {
            T* p = std::addressof(ptr[i][j]);
            ASSERT_TRUE(reinterpret_cast<std::uintptr_t>(p) % alignof(T) == 0);
          }
        }
      }
      {
        using Array = T[][3][2];
        evo::shared_ptr ptr = evo::make_shared<Array>(8);
        for (int i = 0; i < 8; ++i) {
          for (int j = 0; j < 3; ++j) {
            for (int k = 0; k < 2; ++k) {
              T* p = std::addressof(ptr[i][j][k]);
              ASSERT_TRUE(reinterpret_cast<std::uintptr_t>(p) % alignof(T) == 0);
            }
          }
        }
      }
    };

    struct Empty { };
    check_alignment.operator()<Empty>();
    check_alignment.operator()<OverAligned>();
    check_alignment.operator()<MaxAligned>();

    // test non corner cases as well while we're at it
    struct Foo { int i; char c; };
    check_alignment.operator()<int>();
    check_alignment.operator()<Foo>();
  }

  // Make sure that we destroy all the elements constructed so far when an exception
  // is thrown. Also make sure that we do it in reverse order of construction.
  {
    struct Sentinel : ThrowOnConstruction, DestroyInReverseOrder { };

    // Without passing an initial value
    {
      using Array = Sentinel[];
      for (int i = 0; i < 8; ++i) {
        ThrowOnConstruction::throw_after(i);
        DestroyInReverseOrder::reset();
        try {
          evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8);
          ASSERT_TRUE(false);
        } catch (ThrowOnConstruction::exception const&) {
          ASSERT_TRUE(DestroyInReverseOrder::alive() == 0);
        }
      }
    }
    {
      using Array = Sentinel[][3];
      for (int i = 0; i < 8 * 3; ++i) {
        ThrowOnConstruction::throw_after(i);
        DestroyInReverseOrder::reset();
        try {
          evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8);
          ASSERT_TRUE(false);
        } catch (ThrowOnConstruction::exception const&) {
          ASSERT_TRUE(DestroyInReverseOrder::alive() == 0);
        }
      }
    }
    {
      using Array = Sentinel[][3][2];
      for (int i = 0; i < 8 * 3 * 2; ++i) {
        ThrowOnConstruction::throw_after(i);
        DestroyInReverseOrder::reset();
        try {
          evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8);
          ASSERT_TRUE(false);
        } catch (ThrowOnConstruction::exception const&) {
          ASSERT_TRUE(DestroyInReverseOrder::alive() == 0);
        }
      }
    }

    // Passing an initial value
    {
      using Array = Sentinel[];
      for (int i = 0; i < 8; ++i) {
        DestroyInReverseOrder::reset();
        ThrowOnConstruction::reset();
        Sentinel init;
        ThrowOnConstruction::throw_after(i);
        try {
          evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8, init);
          ASSERT_TRUE(false);
        } catch (ThrowOnConstruction::exception const&) {
          ASSERT_TRUE(DestroyInReverseOrder::alive() == 1);
        }
      }
    }
    {
      using Array = Sentinel[][3];
      for (int i = 0; i < 8 * 3; ++i) {
        DestroyInReverseOrder::reset();
        ThrowOnConstruction::reset();
        Sentinel init[3] = {};
        ThrowOnConstruction::throw_after(i);
        try {
          evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8, init);
          ASSERT_TRUE(false);
        } catch (ThrowOnConstruction::exception const&) {
          ASSERT_TRUE(DestroyInReverseOrder::alive() == 3);
        }
      }
    }
    {
      using Array = Sentinel[][3][2];
      for (int i = 0; i < 8 * 3 * 2; ++i) {
        DestroyInReverseOrder::reset();
        ThrowOnConstruction::reset();
        Sentinel init[3][2] = {};
        ThrowOnConstruction::throw_after(i);
        try {
          evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8, init);
          ASSERT_TRUE(false);
        } catch (ThrowOnConstruction::exception const&) {
          ASSERT_TRUE(DestroyInReverseOrder::alive() == 3 * 2);
        }
      }
    }
  }

  // Make sure the version without an initialization argument works even for non-movable types
  {
    using Array = NonMovable[][3];
    evo::shared_ptr<Array> ptr = evo::make_shared<Array>(8);
    (void)ptr;
  }

    // Make sure evo::make_shared handles badly-behaved types properly
    // {
    //   using Array = operator_hijacker[];
    //   evo::shared_ptr<Array> p1 = evo::make_shared<Array>(3);
    //   evo::shared_ptr<Array> p2 = evo::make_shared<Array>(3, operator_hijacker());
    //   ASSERT_TRUE(p1 != nullptr);
    //   ASSERT_TRUE(p2 != nullptr);
    // }

  // Check that we SFINAE-away for invalid arguments
  {
    struct T { };
    static_assert( CanMakeShared<T[], std::size_t>);
    static_assert( CanMakeShared<T[], std::size_t, T>);
    static_assert(!CanMakeShared<T[], std::size_t, T, int>); // too many arguments
  }
}
