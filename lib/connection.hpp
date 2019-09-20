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

#ifndef US3_CONNECTION_HPP_
#define US3_CONNECTION_HPP_

#include "network_socket.hpp"
#include "return_value.hpp"
#include <string>

namespace us3 {

class connection_t {
public:
  /// @brief Stream operation mode.
  enum mode_t {
    NONE = 0,  ///< The stream has not been opened.
    READ = 1,  ///< The stream is open in read mode.
    WRITE = 2  ///< The stream is open in write mode.
  };

  connection_t() : m_mode(NONE), m_socket(NULL) {
  }

  ~connection_t() {
    if (m_mode != NONE) {
      close();
    }
  }

  status_t open(const char* host_name,
                const int port,
                const char* path,
                const char* access_key,
                const char* secret_key,
                const mode_t mode,
                const net::timeout_t connect_timeout,
                const net::timeout_t socket_timeout);

  status_t close();

  result_t<size_t> read(void* buf, const size_t count);

  result_t<size_t> write(const void* buf, const size_t count);

  bool is_connected() const {
    return m_mode != NONE;
  }

private:
  mode_t m_mode;
  net::socket_t m_socket;
  size_t m_content_size;
};

}  // namespace us3

#endif  // US3_CONNECTION_HPP_
