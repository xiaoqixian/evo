// Date:   Wed May 07 12:26:35 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "evo/coroutine/runtime"

namespace evo::coro {

void Runtime::block_on(std::coroutine_handle<> h) {
  if (h.done()) return;

  GLOBAL_CONTEXT.set(&ctx_);
  GLOBAL_DRIVER.set(driver_.get());

  while (!h.done()) {
    while (!ctx_.tasks_.empty()) {
      auto task = std::move(ctx_.tasks_.front());
      ctx_.tasks_.pop_front();
      task.resume();
    }

    driver_->park(-1);
  }
}

} // namespace evo::coro
