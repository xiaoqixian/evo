// Date: Sat Nov 18 11:57:58 2023
// Mail: lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include <coroutine>
#include <exception>
#include <stdio.h>
#include <fmt/format.h>

template <int I>
struct task {
  struct promise_type;
  typedef std::coroutine_handle<promise_type> handle_t;

  handle_t handle;

  struct promise_type {
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

    void return_void() {
      printf("coro%d return_void\n", I);
    }

    void unhandled_exception() {
      std::rethrow_exception(std::current_exception());
    }
  };

  bool await_ready() noexcept {
    printf("coro%d await_ready()\n", I);
    return false;
  }

  void await_suspend(std::coroutine_handle<>) noexcept {
    printf("coro%d await_suspend()\n", I);
  }

  void await_resume() noexcept {
    printf("coro%d await_resume()\n", I);
  }
};

task<2> coro2() {
  printf("coro2 started\n");
  co_return;
}

task<1> coro1() {
  printf("coro1 started\n");
  auto c2 = coro2();
  printf("coro2 created\n");
  co_await c2;
  printf("coro1 resumed\n");
  co_await c2;
  printf("coro1 resumed again\n");
}

int main() {
  auto c1 = coro1();
  fmt::println("coroutine handle size {}", sizeof(c1.handle));
  while (!c1.handle.done()) {
    printf("======= coro1 suspended\n");
    c1.handle.resume();
  }
}

