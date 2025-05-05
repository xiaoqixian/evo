// Date:   Sat May 03 10:25:33 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "evo/coroutine/runtime"
#include <coroutine>
#include <utility>

namespace evo::coro {

template <typename D>
void Runtime<D>::block_on(std::coroutine_handle<> handle) {
  set_poll();
  while (true) {
    while (!ctx_.tasks_.empty()) {
      // set max_round in case of I/O starvation
      auto max_round = ctx_.tasks_.size() * 2;

      while (max_round > 0 && !ctx_.tasks_.empty()) {
        auto task = std::move(ctx_.tasks_.front());
        ctx_.tasks_.pop_front();
        task.resume();
        max_round--;
      }

      while (should_poll()) {
        handle.resume();
        if (handle.done()) {
          return;
        }
      }
    }

    // check for I/O events
    driver_.park();
  }
}

} // namespace evo::coro
