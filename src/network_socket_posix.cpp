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

namespace us3 {
namespace net {

// Platform specific type.
struct socket_struct_t {
  // TODO(m): Implement me!
  int fd;
};

std::pair<socket_t, status::status_t> connect(const char* host,
                                              const int port,
                                              const timeout_t connect_timeout,
                                              const timeout_t socket_timeout) {
  // TODO(m): Implement me!
  return std::make_pair(static_cast<socket_struct_t*>(0), status::ERROR);
}

status::status_t disconnect(socket_t socket) {
  // TODO(m): Implement me!
  return status::ERROR;
}

std::pair<size_t, status::status_t> send(socket_t socket, const void* buf, const size_t count) {
  // TODO(m): Implement me!
  return std::make_pair(0, status::ERROR);
}

std::pair<size_t, status::status_t> recv(socket_t socket, void* buf, const size_t count) {
  // TODO(m): Implement me!
  return std::make_pair(0, status::ERROR);
}

}  // namespace net
}  // namespace us3
