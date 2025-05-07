// Date:   Wed May 07 12:11:02 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "evo/coroutine/context"

namespace evo::coro {

thread_local ThreadLocalWrapper<Context> GLOBAL_CONTEXT = {};

} // namespace evo::coro
