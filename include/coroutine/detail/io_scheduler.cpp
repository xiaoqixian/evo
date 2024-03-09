// Date:   Fri Mar 08 16:19:43 2024
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "coroutine/io_scheduler.hpp"
#include "coroutine/task.hpp"
#include <iostream>

namespace evo {

#ifdef __APPLE__

#include <sys/event.h>
#include <sys/time.h>

/*
* int kevent(
           int kq,	   
           const struct	kevent *changelist,	 
           int nchanges,
           struct kevent *eventlist,				  
           int nevents,
           const struct	timespec *timeout
      );
  EV_SET(kev, ident, filter, flags, fflags, data, udata);
*/
evo::task<poll_status> 
io_scheduler::poll(int fd, poll_op op, io_scheduler::timeunit_t timeout) {
    poll_info pi(fd);

    struct kevent ev[2] {};;

    // add fd to kqueue
    EV_SET(&ev[0], fd, static_cast<int16_t>(op), 
            EV_ADD | EV_CLEAR | EV_ONESHOT, 0, 0, 
            static_cast<void*>(&pi));

    // add a timer
    EV_SET(&ev[1], fd, EVFILT_TIMER, 
            EV_ADD | EV_CLEAR | EV_ONESHOT, 
            0, timeout.count(),
            static_cast<void*>(&pi));

    int ret = kevent(this->kq_fd, ev, 2, NULL, 0, NULL);
    if (ret == -1) {
        std::cerr << "kevent() failed" << std::endl;
        std::exit(1);
    }

    auto res = co_await pi;
    co_return res;
}

void io_scheduler::run(io_scheduler::timeunit_t timeout) {
    struct timespec t { 0, timeout.count() };;
    
    const int n_events = kevent(
        this->kq_fd,
        NULL,
        0,
        this->events.data(),
        MAX_EVENTS,
        &t
    );

    for (std::size_t i = 0; i < static_cast<std::size_t>(n_events); i++) {
        struct kevent& ev = this->events[i];

        poll_status status;

        // generate poll status from event flags.
        if (ev.flags & EV_ERROR) {
            status = poll_status::ERROR;
        }
        else if (ev.flags & EV_EOF) {
            status = poll_status::CLOSED;
        }
        else if (ev.filter & EVFILT_TIMER) {
            status = poll_status::TIMEOUT;
            // TODO delete timeout events.
        } else {
            status = poll_status::EVENT;
        }

        poll_info* pip = static_cast<poll_info*>(ev.udata);
        pip->status = status;

        this->handles_to_resume.emplace_back(pip->handle);
    }

    for (auto& h: this->handles_to_resume) {
        // TODO: handle resume policy.
        h.resume();
    }
    this->handles_to_resume.clear();
}

#else // TODO: work for Unix platform

#endif // __APPLE__

} // namespace evo
