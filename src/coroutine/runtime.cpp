// Date:   Wed May 07 12:26:35 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "evo/coroutine/runtime"

namespace evo::coro {

void RuntimeImpl::block_on(std::coroutine_handle<> h) {
  if (h.done()) return;

  h.resume();

  while (true) {
    while (!ctx_.tasks_.empty()) {
      auto task = std::move(ctx_.tasks_.front());
      ctx_.tasks_.pop_front();
      task.run();
    }

    if (h.done()) return;

    driver_->park(-1);
  }
}

thread_local std::unique_ptr<RuntimeImpl> Runtime::thread_runtime;

void Runtime::block_on(std::coroutine_handle<> h) {
  thread_runtime->block_on(h);
}

} // namespace evo::coro
