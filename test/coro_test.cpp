// Date: Sat Nov 18 11:57:58 2023
// Mail: lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include <coroutine>
#include <exception>
#include <future>
#include <stdio.h>
#include <fmt/format.h>

template <int I, typename T>
struct task {
  struct promise_type;
  typedef std::coroutine_handle<promise_type> handle_t;

  handle_t handle;

  struct promise_type {
    T value_;

    auto get_return_object() {
      printf("coro%d get_return_object\n", I);
      return task {
        handle_t::from_promise(*this)
      };
    }

    auto initial_suspend() {
      printf("coro%d initial_suspend\n", I);
      return std::suspend_always();
    }

    auto final_suspend() noexcept {
      printf("coro%d final_suspend\n", I);
      return std::suspend_always();
    }

    void return_value(T const& value) {
      printf("coro%d return_value const&\n", I);
      value_ = value;
    }

    void return_value(T && value) {
      printf("coro%d return_value &&\n", I);
      value_ = std::move(value);
    }

    void unhandled_exception() {
      std::rethrow_exception(std::current_exception());
    }
  };

  bool await_ready() noexcept {
    printf("coro%d await_ready()\n", I);
    return false;
  }

  bool await_suspend(std::coroutine_handle<>) noexcept {
    printf("coro%d await_suspend()\n", I);
    return false;
  }

  auto await_resume() noexcept {
    printf("coro%d await_resume()\n", I);
    return 1;
  }
};

task<2, int> coro2() {
  printf("coro2 started\n");
  co_return 2;
}

task<1, int> coro1() {
  printf("coro1 started\n");
  auto c2 = coro2();
  printf("coro2 created\n");
  auto res = co_await c2;
  printf("coro1 resumed\n");
  co_await c2;
  printf("coro1 resumed again\n");
  co_return 1;
}

int main() {
  auto c1 = coro1();

  while (!c1.handle.done()) {
    printf("======= coro1 suspended\n");
    c1.handle.resume();
  }
}

