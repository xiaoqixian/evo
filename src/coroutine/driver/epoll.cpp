// Date:   Sun May 04 21:40:13 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "evo/coroutine/driver"
#include <sys/epoll.h>
#include "evo/macros"

namespace evo::coro {

EpollDriver::EpollDriver()
  : Driver(DriverType::Epoll)
{
  epfd_ = ::epoll_create1(0);
  if (epfd_ == -1) {
    SYS_ERROR(epoll_create1);
  }
}

void EpollDriver::park(int timeout) {
  ::epoll_event events[1024];
  int res = ::epoll_wait(epfd_, events, 1024, timeout);
  if (res == -1) {
    SYS_ERROR(epoll_wait);
  }
  for (int i = 0; i < res; i++) {
    const int fd = events[i].data.fd;
    const u32 ev = events[i].events;
    auto& io = io_dispatch_[fd];
    
    io.wake(Readiness::from_epoll_events(ev));
  }
}

void EpollDriver::register_fd(int fd) {
  ::epoll_event ev;
  ev.data.fd = fd;
  ev.events = EPOLLET | EPOLLIN | EPOLLOUT;
  if (::epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
    SYS_ERROR(epoll_ctl);
  }
  ScheduledIO io;
  io.set_readiness([](Readiness) {return Readiness::READABLE | Readiness::WRITABLE;});
  io_dispatch_.emplace(fd, std::move(io));
}

void EpollDriver::deregister_fd(int fd) {
  if (::epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) == -1) {
    SYS_ERROR(epoll_ctl);
  }
  io_dispatch_.erase(fd);
}

} // namespace evo::coro
