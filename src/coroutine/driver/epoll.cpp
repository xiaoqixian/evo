// Date:   Sun May 04 21:40:13 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "evo/coroutine/driver"
#include <sys/epoll.h>
#include "evo/macros"
#include "evo/debug"

namespace evo::coro {

EpollDriver::EpollDriver() {
  epfd_ = ::epoll_create1(0);
  if (epfd_ == -1) {
    SYS_ERROR(epoll_create1);
  }
}

void EpollDriver::suspend(Idx idx, IODirection direction, std::coroutine_handle<> h) {
  auto& scheduled_io = io_scheduler_[idx];

  if (direction.is_write()) {
    scheduled_io.writer_ = h;
  }
  else {
    scheduled_io.reader_ = h;
  }
}

bool EpollDriver::check_ready(Idx idx, IODirection direction, std::coroutine_handle<> h) {
  return io_scheduler_[idx].check_ready(direction, h);
}

void EpollDriver::register_fd(int fd) {
  io_scheduler_.emplace(fd, ScheduledIO {});
  ::epoll_event ev {};
  ev.data.fd = fd;
  ev.events = EPOLLIN | EPOLLOUT;
  if (::epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
    SYS_ERROR(epoll_ctl);
  }
}

void EpollDriver::deregister_fd(int fd) {
  DEBUG_ASSERT(io_scheduler_.contains(fd), "");
  io_scheduler_.erase(fd);

  if (::epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) == -1) {
    SYS_ERROR(epoll_ctl);
  }
}

void EpollDriver::park(Duration timeout) {
  ::epoll_event events[1024];
  int nfds = ::epoll_wait(epfd_, events, 1024, timeout.count());

  if (nfds == -1) {
    SYS_ERROR(epoll_wait);
  }

  for (int i = 0; i < nfds; i++) {
    auto& scheduled_io = io_scheduler_[events[i].data.fd];
    auto ready = Ready::from_epoll_events(events[i].events);
    scheduled_io.set_readiness([ready](auto curr) {return curr | ready;});
    scheduled_io.wake(ready);
  }
}

} // namespace evo::coro
