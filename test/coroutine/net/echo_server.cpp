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

task<> handler(net::TcpStream stream) {
  fmt::println("handler start");
  char buf[1024] {};
  int rb = co_await stream.read(buf, 1024);
  printf("Cli send: %s\n", buf);
  constexpr const char* RESP = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\nConnection: keep-alive\r\n\r\nHello, World!";
  co_await stream.write(RESP, std::strlen(RESP));
  fmt::println("handler return");
}

task<> server() {
  auto listener = net::TcpListener::bind(9900);
  fmt::println("TCP listener created");

  auto stream = co_await listener.accept();
  fmt::println("Stream accepted");

  auto handle = Runtime::spawn(handler(std::move(stream)));
  fmt::println("handler spawn");
  co_await handle;
  fmt::println("handler joined");
  // while (true) {
  //   auto stream = co_await listener.accept();
  //   fmt::println("Accept a connection");
  //   char buf[1024] {};
  //   int rb = co_await stream.read(buf, 1024);
  //   printf("Cli send: %s\n", buf);
  //   const char* resp = "Server hello";
  //   co_await stream.write(resp, std::strlen(resp));
  // }
}

int main() {
  Runtime::init();

  auto srv = server();
  Runtime::block_on(srv.handle());
}
