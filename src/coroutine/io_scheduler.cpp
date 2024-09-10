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

#else 
#include <sys/epoll.h>
#endif

#ifdef __APPLE__
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
io_scheduler_base::poll(int fd, poll_op op, io_scheduler_base::timeunit_t timeout) {
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

evo::task<poll_status> 
io_scheduler_base::poll(int fd, poll_op op) {
    poll_info pi(fd);

    struct kevent ev;

    // add fd to kqueue
    EV_SET(&ev, fd, static_cast<int16_t>(op), 
            EV_ADD | EV_CLEAR | EV_ONESHOT, 0, 0, 
            static_cast<void*>(&pi));

    int ret = kevent(this->kq_fd, &ev, 1, NULL, 0, NULL);
    if (ret == -1) {
        std::cerr << "kevent() failed" << std::endl;
        std::exit(1);
    }

    auto res = co_await pi;
    co_return res;
}

void io_scheduler_base::run(io_scheduler_base::timeunit_t timeout) {
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

#else // for linux platform

evo::task<poll_status>
io_scheduler_base::poll(int fd, poll_op op, io_scheduler_base::timeunit_t timeout) {
    poll_info pi(fd);

    epoll_event e {};
    e.events = static_cast<uint32_t>(op) | EPOLLONESHOT | EPOLLRDHUP;
    e.data.ptr = &pi;

    if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, fd, &e) == -1) 
        std::cerr << "epoll_ctl error on fd " << fd << std::endl;

    auto res = co_await pi;
    co_return res;
}

void io_scheduler_base::run(io_scheduler_base::timeunit_t timeout) {
    const int event_count = 
        epoll_wait(this->epoll_fd, this->events.data(), io_scheduler_base::MAX_EVENTS, timeout.count());

    for (std::size_t i = 0; i < event_count; i++) {
        epoll_event const& ev = this->events[i];

        poll_status status;
        uint32_t ev_flags = ev.events;

        if (ev_flags & EPOLLIN || ev_flags & EPOLLOUT) 
            status = poll_status::EVENT;
        else if (ev_flags & EPOLLERR)
            status = poll_status::ERROR;
        else if (ev_flags & EPOLLRDHUP || ev_flags & EPOLLHUP) 
            status = poll_status::CLOSED;
        else 
            throw std::runtime_error("invalid event status");

        poll_info* pip = static_cast<poll_info*>(ev.data.ptr);
        pip->status = status;

        // del event from epoll
        if (pip->fd != -1) {
            epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, pip->fd, nullptr);
        }

        this->handles_to_resume.emplace_back(pip->handle);
    }

    this->resume_handles();
    // for (auto& h: this->handles_to_resume) {
    //     // TODO: handle resume policy
    //     h.resume();
    // }
    //
    this->handles_to_resume.clear();
}

#endif // __APPLE__

template <typename ThreadPoolType>
io_scheduler<ThreadPoolType>::io_scheduler(std::shared_ptr<ThreadPoolType> p)
    : pool(p) {}

template <typename ThreadPoolType>
void io_scheduler<ThreadPoolType>::resume_handles() {
    for (auto& h: this->handles_to_resume) {
        this->pool->resume(h);
    }
}

void io_scheduler<void>::resume_handles() {
    for (auto& h: this->handles_to_resume) {
        h.resume();
    }
}

} // namespace evo
