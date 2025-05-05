// Date:   Sun May 04 20:15:27 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include <sys/socket.h>
#include <unistd.h>
#include <fmt/format.h>
#include "evo/coroutine/net/tcp/listener"
#include "evo/coroutine/net/tcp/accept"

namespace evo::coro::net {

TcpListener TcpListener::bind(int port) {
  const int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
  if (fd == -1) {
    throw std::runtime_error(std::string(std::strerror(errno)));
  }

  sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = port;
  
  if (::bind(fd, (sockaddr*)&addr, sizeof(addr)) == -1) {
    throw std::runtime_error(fmt::format("Failed to bind port {}: {}", port, std::strerror(errno)));
  }

  if (::listen(fd, BACKLOG) == -1) {
    throw std::runtime_error("Listen failed");
  }

  return TcpListener {fd};
}

task<TcpStream> TcpListener::accept() {
  const int fd = co_await OpAccept::accept(fd_);
  co_return TcpStream(fd);
}

} // namespace evo::coro::net
