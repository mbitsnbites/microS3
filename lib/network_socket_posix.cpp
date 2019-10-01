//--------------------------------------------------------------------------------------------------
// Copyright (c) 2019 Marcus Geelnard
//
// This software is provided 'as-is', without any express or implied warranty. In no event will the
// authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including commercial
// applications, and to alter it and redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not claim that you wrote
//     the original software. If you use this software in a product, an acknowledgment in the
//     product documentation would be appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and must not be misrepresented as
//     being the original software.
//
//  3. This notice may not be removed or altered from any source distribution.
//--------------------------------------------------------------------------------------------------

#include "network_socket.hpp"

#include <cstdio>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace us3 {
namespace net {

// Platform specific type.
struct socket_struct_t {
  int fd;
};

namespace {

#if __cplusplus >= 201103L
const socket_t NULL_SOCKET_T(nullptr);
#else
const socket_t NULL_SOCKET_T(NULL);
#endif

status_t::status_enum_t errno_to_status() {
  switch (errno) {
    case EACCES:
      return status_t::DENIED;
    case ECONNREFUSED:
      return status_t::REFUSED;
    case ENETUNREACH:
      return status_t::UNREACHABLE;
    case ECONNRESET:
      return status_t::CONNECTION_RESET;
    case ETIMEDOUT:
      return status_t::TIMEOUT;
    default:
      return status_t::ERROR;
  }
}

}  // namespace

result_t<socket_t> connect(const char* host,
                           const int port,
                           const timeout_t connect_timeout,
                           const timeout_t socket_timeout) {
  // TODO(m): Make use of these arguments.
  (void)connect_timeout;
  (void)socket_timeout;

  // Get address info for the host / port.
  ::addrinfo* info;
  {
    ::addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    char port_str[30];
    std::sprintf(&port_str[0], "%d", port);
    if (::getaddrinfo(host, port_str, &hints, &info) != 0) {
      return make_result(NULL_SOCKET_T, status_t::NO_HOST);
    }
  }

  // Open the socket.
  const int socket_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket_fd == -1) {
    ::freeaddrinfo(info);
    return make_result(NULL_SOCKET_T, errno_to_status());
  }

  // Connect to the host.
  // TODO(m): Implement timeout. See e.g. https://stackoverflow.com/a/2597774/5778708
  if (::connect(socket_fd, info->ai_addr, info->ai_addrlen) == -1) {
    ::close(socket_fd);
    ::freeaddrinfo(info);
    return make_result(NULL_SOCKET_T, errno_to_status());
  }

  // Return the socket handle.
  socket_t new_socket = new socket_struct_t();
  new_socket->fd = socket_fd;
  return make_result(new_socket, status_t::SUCCESS);
}

status_t disconnect(socket_t socket) {
  ::close(socket->fd);
  delete socket;
  return make_result(status_t::SUCCESS);
}

result_t<size_t> send(socket_t socket, const void* buf, const size_t count) {
  const ssize_t actual_count = ::send(socket->fd, buf, count, 0);
  if (actual_count == -1) {
    return make_result<size_t>(0, errno_to_status());
  }
  return make_result(static_cast<size_t>(actual_count), status_t::SUCCESS);
}

result_t<size_t> recv(socket_t socket, void* buf, const size_t count) {
  const ssize_t actual_count = ::recv(socket->fd, buf, count, 0);
  if (actual_count == -1) {
    return make_result<size_t>(0, errno_to_status());
  }
  return make_result(static_cast<size_t>(actual_count), status_t::SUCCESS);
}

}  // namespace net
}  // namespace us3
