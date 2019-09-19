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

#ifndef US3_NETWORK_SOCKET_HPP_
#define US3_NETWORK_SOCKET_HPP_

#include "return_value.hpp"
#include <cstddef>

namespace us3 {
namespace net {

// Forward declaration. This is implementation defined.
typedef struct socket_struct_t* socket_t;
struct socket_struct_t;

/// @brief Timeout in microseconds.
typedef long timeout_t;

/// @brief Establish a socket connection.
std::pair<socket_t, status::status_t> connect(const char* host,
                                              const int port,
                                              const timeout_t connect_timeout,
                                              const timeout_t socket_timeout);

/// @brief Close a socket connection.
status::status_t disconnect(socket_t socket);

/// @brief Send data over a socket.
std::pair<size_t, status::status_t> send(socket_t socket, const void* buf, const size_t count);

/// @brief Receive data over a socket.
std::pair<size_t, status::status_t> recv(socket_t socket, void* buf, const size_t count);

}  // namespace net
}  // namespace us3

#endif  // US3_NETWORK_SOCKET_HPP_
