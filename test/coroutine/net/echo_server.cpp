// Date:   Wed May 07 16:27:57 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "evo/coroutine/net/tcp/listener"
#include "evo/coroutine/runtime"
#include "evo/coroutine/task"
#include "evo/types"
#include "evo/coroutine/op"
#include "fmt/core.h"
#include <cstdio>
#include <cstring>
#include <arpa/inet.h>

using evo::u8;
using namespace evo::coro;

task<> server() {
  auto listener = net::TcpListener::bind(9900);
  fmt::println("TCP listener created");

  while (true) {
    auto stream = co_await listener.accept();
    fmt::println("Accept a connection");
    char buf[1024] {};
    int rb = co_await stream.read(buf, 1024);
    printf("Cli send: %s\n", buf);
    const char* resp = "Server hello";
    co_await stream.write(resp, std::strlen(resp));
  }
}

int main() {
  // int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  // if (server_fd == -1) {
  //     perror("socket");
  //     return 1;
  // }
  //
  // sockaddr_in addr{};
  // addr.sin_family = AF_INET;
  // addr.sin_port = htons(9900);
  // addr.sin_addr.s_addr = INADDR_ANY;
  //
  // // 绑定地址
  // if (bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
  //     perror("bind");
  //     close(server_fd);
  //     return 1;
  // }
  //
  // // 监听端口
  // if (listen(server_fd, 5) == -1) {
  //     perror("listen");
  //     close(server_fd);
  //     return 1;
  // }
  //
  // while (true) {
  //   int res = accept(server_fd, nullptr, nullptr);
  //   printf("accept one %d\n", res);
  // }
  auto runtime = Runtime(EpollDriverTag {});

  auto srv = server();
  runtime.block_on(srv.handle());
}
