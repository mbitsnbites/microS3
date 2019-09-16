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

#include "connection.hpp"

namespace us3 {

status::status_t connection_t::open(const char* host_name,
                                    const int port,
                                    const char* path,
                                    const char* access_key,
                                    const char* secret_key,
                                    const connection_t::mode_t mode,
                                    const connection_t::timeout_t timeout) {
  // Sanity check arguments.
  // Note: All pointers are guaranteed to be non-NULL at this point.
  if (port < 1 || port > 65535) {
    return status::INVALID_OPERATION;
  }

  // We must not open a connection that is already opened.
  if (m_mode != NONE) {
    return status::INVALID_OPERATION;
  }

  // TODO(m): Implement me!
  return status::ERROR;
}

status::status_t connection_t::close() {
  // We can not close a connection that is already closed.
  if (m_mode == NONE) {
    return status::INVALID_OPERATION;
  }

  // TODO(m): Implement me!
  return status::ERROR;
}

status::status_t connection_t::read(void* buf,
                                    const size_t count,
                                    const connection_t::timeout_t timeout,
                                    size_t& actual_count) {
  // The connection must have been opened in read mode.
  if (m_mode != READ) {
    return status::INVALID_OPERATION;
  }

  // TODO(m): Implement me!
  return status::ERROR;
}

status::status_t connection_t::write(const void* buf,
                                     const size_t count,
                                     const connection_t::timeout_t timeout,
                                     size_t& actual_count) {
  // The connection must have been opened in write mode.
  if (m_mode != WRITE) {
    return status::INVALID_OPERATION;
  }

  // TODO(m): Implement me!
  return status::ERROR;
}

}  // namespace us3
