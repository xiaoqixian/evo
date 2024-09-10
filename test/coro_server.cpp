// Date:   Mon Mar 18 10:16:42 2024
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "coroutine/io_scheduler.hpp"
#include "coroutine/task.hpp"
#include "coroutine/static_thread_pool.hpp"
#include "coroutine/when_all.hpp"
#include "debug.h"
#include "coroutine/sync_wait.hpp"
#include "coroutine/traits.hpp"

#include <chrono>
#include <coroutine>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

template <typename Pool, typename Scheduler>
evo::task<> client_handler(const int client_fd, Scheduler& scheduler) {
    char buf[1024];

    while (true) {
        auto status = co_await scheduler.poll(client_fd, evo::poll_op::READ);
        ASSERT(status == evo::poll_status::EVENT, "");

        memset(buf, '\0', 1024);
        int read_size = read(client_fd, buf, 1024);
        
        ASSERT(read_size > 0, "read failed");

        printf("recv: %s\n", buf);
    }
}

evo::task<void> create_socket() {
    int ret;

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(struct sockaddr_in));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(8080);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT(listen_fd >= 0, "create socket failed");

    int optval = 1;
    ret = setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof optval);

    ASSERT(ret >= 0, "setsockopt failed");

    if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0) {
        LOG_ERR("bind failed");
    }

    if (listen(listen_fd, 1024) < 0) {
        LOG_ERR("listen failed");
    }

    auto pool = std::make_shared<evo::static_thread_pool>(
        evo::static_thread_pool());

    evo::io_scheduler<evo::static_thread_pool> scheduler(pool);

    while (true) {
        co_await scheduler.poll(listen_fd, evo::poll_op::READ);

        struct sockaddr_in client_addr;
        socklen_t len;
        const int client_fd = 
            accept(listen_fd, (struct sockaddr*)&client_addr, &len);

        ASSERT(client_fd >= 0, "accept client fd failed");
        
    }
}

