// Date:   Wed May 07 16:37:28 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "evo/coroutine/net/tcp/stream"
#include "evo/coroutine/task"
#include "evo/coroutine/op"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

namespace evo::coro::net {

task<int> TcpStream::read(char* buf, size_t size) {
  auto res = co_await Op::read(fd_, buf, size);
  co_return res.unwrap_or_throw();
}

task<int> TcpStream::write(char const* buf, size_t size) {
  auto res = co_await Op::write(fd_, buf, size);
  co_return res.unwrap_or_throw();
}

task<TcpStream> TcpStream::connect(const char* addr, u16 port) {
  ::sockaddr_in srv_addr;
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = ::htons(port);
  if (::inet_pton(AF_INET, addr, &srv_addr.sin_addr) <= 0) {
    SYS_ERROR(inet_pton);
  }

  return TcpStream::connect(srv_addr);
}

task<TcpStream> TcpStream::connect(::sockaddr_in addr) {
  const int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  if (sockfd == -1) {
    SYS_ERROR(socket);
  }

  Fd fd(sockfd);

  auto res = co_await Op::connect(fd, reinterpret_cast<::sockaddr*>(&addr));

  res.unwrap_or_throw();

  co_return TcpStream(std::move(fd));
}

} // namespace evo::coro::net
