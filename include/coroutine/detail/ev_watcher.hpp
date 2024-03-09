// Date:   Fri Mar 08 11:33:00 2024
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#ifndef _EV_WATCHER_HPP
#define _EV_WATCHER_HPP

#include <array>

namespace evo {

namespace detail {

#ifdef __APPLE__

#include <sys/event.h>

enum class poll_op {
    READ = EVFILT_READ,
    WRITE = EVFILT_WRITE
};

class poll_watcher {
    int kq = -1;
    static const constexpr int max_events = 16;
    std::array<struct kevent, max_events> events;

    poll_watcher() {
        this->kq = kqueue();
    }

    
};

#endif

} // namespace detail

} // namespace evo

#endif // _EV_WATCHER_HPP
