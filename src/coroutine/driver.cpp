// Date:   Sun May 04 16:04:56 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "evo/coroutine/driver"

namespace evo::coro {

thread_local ThreadLocalWrapper<Driver> GLOBAL_DRIVER = {};

void Driver::park(int timeout) {
  DRIVER_PROXY(park, timeout);
}

void Driver::register_fd(int fd) {
  DRIVER_PROXY(register_fd, fd);
}

void Driver::deregister_fd(int fd) {
  DRIVER_PROXY(deregister_fd, fd);
}


} // namespace evo::coro
