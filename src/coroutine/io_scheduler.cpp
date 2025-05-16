// Date:   Wed May 07 18:09:16 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "evo/coroutine/io_scheduler"
#include <optional>

namespace evo::coro {

bool ScheduledIO::check_ready(IODirection direction, Waker waker) {
  if (direction.is_write()) {
    if (readiness_.is_writable()) {
      return true;
    } else {
      writer_.emplace(std::move(waker));
      return false;
    }
  } else {
    if (readiness_.is_readable()) {
      return true;
    } else {
      reader_.emplace(std::move(waker));
      return false;
    }
  }
}

void ScheduledIO::set_waker(IODirection direction, Waker waker) {
  if (direction.is_write()) {
    writer_.emplace(std::move(waker));
  } else {
    reader_.emplace(std::move(waker));
  }
}

void ScheduledIO::wake(Readiness readiness) {
  fmt::println("readable: {}, has reader: {}", readiness.is_readable(), reader_.has_value());
  fmt::println("writable: {}, has writer: {}", readiness.is_writable(), writer_.has_value());
  if (readiness.is_readable() && reader_.has_value()) {
    auto waker = std::move(reader_.value());
    reader_.reset();
    waker.wake();
  }
  if (readiness.is_writable() && writer_.has_value()) {
    auto waker = std::move(writer_.value());
    writer_.reset();
    waker.wake();
  }
}

} // namespace evo::coro
