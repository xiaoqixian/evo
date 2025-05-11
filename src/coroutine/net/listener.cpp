// Date:   Sun May 04 20:15:27 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fmt/format.h>
#include "evo/coroutine/net/tcp/listener"
#include "evo/coroutine/op"
#include "evo/macros"

namespace evo::coro::net {

TcpListener TcpListener::bind(u16 port) {
  const int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
  if (fd == -1) {
    throw std::runtime_error(std::string(std::strerror(errno)));
  }

  ::sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
  addr.sin_port = ::htons(port);
  
  // int res = ::bind(fd, (sockaddr*)&addr, sizeof(addr));
  // printf("bind res = %d\n", res);
  if (::bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
    ERROR("Failed to bind port {}: {}", port, std::strerror(errno));
  }

  printf("bind on fd %d\n", fd);

  // res = ::listen(fd, BACKLOG);
  // printf("listen res = %d\n", res);
  if (::listen(fd, BACKLOG) < 0) {
    SYS_ERROR(listen);
  }

  return TcpListener {fd};
}

task<TcpStream> TcpListener::accept() {
  auto res = co_await Op::accept(fd_);
  const int fd = res.unwrap_or_throw();
  co_return TcpStream(fd);
}

} // namespace evo::coro::net
