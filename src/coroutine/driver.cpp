// Date:   Sun May 04 16:04:56 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "evo/coroutine/driver"

namespace evo::coro {

thread_local std::unique_ptr<Driver> GLOBAL_DRIVER = nullptr;

} // namespace evo::coro
