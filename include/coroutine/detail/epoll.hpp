// Date:   Fri Mar 08 10:33:40 2024
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#ifndef _EPOLL_HPP
#define _EPOLL_HPP

#include <iostream>

namespace evo {

namespace epoll_detail {

// A struct to describe a file descriptor
// and its associated data ptr.
struct poll_info {
    int fd;
    void* data;
};

#ifdef __APPLE__

#include <sys/event.h>

enum epoll_op {
    EPOLL_READ = EVFILT_READ,
    EPOLL_WRITE = EVFILT_WRITE,
};

/*
 * int
       kevent(int kq,	   const struct	kevent *changelist,	 int nchanges,
	   struct kevent *eventlist,				  int nevents,
	   const struct	timespec *timeout);
 */

class epoll {
    int kq_id = -1;
    
    epoll() {
        this->kq_id = kqueue();
        if (this->kq_id == -1) {
            std::cerr << "kqueue() failed" << std::endl;
            std::exit(1);
        }
    }

    bool epoll_add(int fd, epoll_op op, void* data) {
        struct kevent ev;
        EV_SET(&ev, fd, op, EV_ADD | EV_CLEAR, 0, 0, data);

        // attach the event to the kqueue
        const int ret = kevent(this->kq_id, &ev, 1, NULL, 0, NULL);
        if (ret == -1) {
            std::cerr << "kevent() attach an event failed" << std::endl;
            return false;
        }
        return true;
    }

};

#endif

}

}

#endif // _EPOLL_HPP
