// Date:   Thu May 15 02:51:47 PM 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "evo/coroutine/net/tcp/listener"
#include "evo/coroutine/net/tcp/stream"
#include "evo/coroutine/runtime"
#include "fmt/core.h"
#include <cstdio>
#include <cstring>

using namespace evo::coro;

task<> handler(net::TcpStream stream) {
  fmt::println("Accept a new client");
  char buf[1024];
  const char* RESP = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\nConnection: keep-alive\r\n\r\nHello, World!";
  while (true) {
    int bytes = co_await stream.read(buf, 1024);
    if (bytes < 0) {
      printf("read error\n");
      exit(1);
    }
    if (bytes == 0) break;
    
    co_await stream.write(RESP, std::strlen(RESP));
  }
}

task<> srv() {
  auto listener = net::TcpListener::bind(9090);
  
  while (true) {
    auto stream = co_await listener.accept();
    Runtime::spawn(handler(std::move(stream)));
  }
}

int main() {
  Runtime::init();
  auto server = srv();
  Runtime::block_on(server.handle());
}
