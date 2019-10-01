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

#include <cstddef>
#include <cstdio>
#include <winsock2.h>
#include <ws2tcpip.h>
#undef ERROR

namespace us3 {
namespace net {

// Platform specific type.
struct socket_struct_t {
  SOCKET handle;
};

namespace {

#if __cplusplus >= 201103L
const socket_t NULL_SOCKET_T(nullptr);
#else
const socket_t NULL_SOCKET_T(NULL);
#endif

bool s_wsa_initialized = false;

bool wsa_initialize() {
  if (s_wsa_initialized) {
    return true;
  }
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    return false;
  }
  s_wsa_initialized = true;
  return true;
}

status_t::status_enum_t wsa_error_to_status(const int err) {
  switch (err) {
    case WSAEACCES:
      return status_t::DENIED;
    case WSAECONNREFUSED:
      return status_t::REFUSED;
    case WSAENETUNREACH:
      return status_t::UNREACHABLE;
    case WSAECONNRESET:
      return status_t::CONNECTION_RESET;
    case WSAETIMEDOUT:
      return status_t::TIMEOUT;
    default:
      return status_t::ERROR;
  }
}

status_t::status_enum_t wsa_error_to_status() {
  return wsa_error_to_status(WSAGetLastError());
}

}  // namespace

result_t<socket_t> connect(const char* host,
                           const int port,
                           const timeout_t connect_timeout,
                           const timeout_t socket_timeout) {
  // TODO(m): Make use of these arguments.
  (void)connect_timeout;
  (void)socket_timeout;

  if (!wsa_initialize()) {
    return make_result(NULL_SOCKET_T, status_t::ERROR);
  }

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
  const SOCKET socket_handle = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket_handle == INVALID_SOCKET) {
    ::freeaddrinfo(info);
    return make_result(NULL_SOCKET_T, wsa_error_to_status());
  }

  // Connect to the host.
  // TODO(m): Implement timeout. See e.g. https://stackoverflow.com/a/2597774/5778708
  if (::connect(socket_handle, info->ai_addr, static_cast<int>(info->ai_addrlen)) == -1) {
    ::closesocket(socket_handle);
    ::freeaddrinfo(info);
    return make_result(NULL_SOCKET_T, wsa_error_to_status());
  }

  // Return the socket handle.
  socket_t new_socket = new socket_struct_t();
  new_socket->handle = socket_handle;
  return make_result(new_socket, status_t::SUCCESS);
}

status_t disconnect(socket_t socket) {
  ::closesocket(socket->handle);
  delete socket;
  return make_result(status_t::SUCCESS);
}

result_t<size_t> send(socket_t socket, const void* buf, const size_t count) {
  const int actual_count =
      ::send(socket->handle, reinterpret_cast<const char*>(buf), static_cast<int>(count), 0);
  if (actual_count == -1) {
    return make_result<size_t>(0, wsa_error_to_status());
  }
  return make_result(static_cast<size_t>(actual_count), status_t::SUCCESS);
}

result_t<size_t> recv(socket_t socket, void* buf, const size_t count) {
  const int actual_count =
      ::recv(socket->handle, reinterpret_cast<char*>(buf), static_cast<int>(count), 0);
  if (actual_count == -1) {
    return make_result<size_t>(0, wsa_error_to_status());
  }
  return make_result(static_cast<size_t>(actual_count), status_t::SUCCESS);
}

}  // namespace net
}  // namespace us3
