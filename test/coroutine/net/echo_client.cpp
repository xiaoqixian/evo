// Date:   Wed May 07 17:42:44 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "evo/coroutine/net/tcp/stream"
#include "evo/coroutine/runtime"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace evo::coro;

task<> client() {
  auto stream = co_await net::TcpStream::connect("127.0.0.1", 9900);
  fmt::println("Connection established");
  char buf[1024] {};
  const char* req = "hello client";
  co_await stream.write(req, std::strlen(req));
  fmt::println("Sent 'hello client'");
  int rb = co_await stream.read(buf, 1024);
  printf("Srv send: %s\n", buf);
}

int main() {
  auto runtime = Runtime(EpollDriverTag {});
  runtime.block_on(client().handle());
}
