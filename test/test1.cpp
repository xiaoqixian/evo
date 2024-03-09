// Date: Sat Nov 18 11:57:58 2023
// Mail: lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include <sys/event.h>
#include <sys/time.h>
#include <stdio.h>
#include <thread>

int main() {
    int kq = kqueue();
    int ret;

    const char* msg = "information";

    struct kevent kev;
    // EV_SET(&kev, 123, EVFILT_READ, EV_ADD | EV_CLEAR | EV_ONESHOT, 0, 0, NULL);
    EV_SET(&kev, 0, EVFILT_TIMER, EV_ADD | EV_CLEAR | EV_ONESHOT, 0, 3000, (void*)msg);

    ret = kevent(kq, &kev, 1, NULL, 0, NULL);
    if (ret == -1) 
        printf("add event failed\n");

    // struct kevent del;
    // EV_SET(&del, 0, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
    // kevent(kq, &del, 1, NULL, 0, NULL);

    struct kevent res;
    struct timespec timeout { 5, 0 };

    printf("before wait\n");
    ret = kevent(kq, NULL, 0, &res, 1, &timeout);
    printf("ret = %d\n", ret);
    if (ret > 0) {
        printf("msg: %s\n", (char*)res.udata);
    }

    if (res.filter & EVFILT_TIMER) {
        printf("this is a timer\n");
    }

    // while (true) {
    //     std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //     int ret = kevent(kq, NULL, 0, &res, 1, NULL);
    //     if (ret > 0) {
    //         printf("get triggerd\n");
    //         break;
    //     }
    // }

    printf("after wait\n");
}
