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
#include "network_socket.hpp"
#include "return_value.hpp"
#include "url_parser.hpp"
#include <cstring>
#include <us3/us3.h>

struct us3_handle_struct_t {
  us3::connection_t connection;
};

namespace {
bool is_valid_handle(const us3_handle_t& handle) {
  // TODO(m): Do a more robust check.
  return handle != NULL;
}

us3_status_t to_capi_status(const us3::status::status_t result) {
  switch (result) {
    case us3::status::SUCCESS:
      return US3_SUCCESS;
    case us3::status::INVALID_ARGUMENT:
      return US3_INVALID_ARGUMENT;
    case us3::status::INVALID_OPERATION:
      return US3_INVALID_OPERATION;
    case us3::status::INVALID_URL:
      return US3_INVALID_URL;
    case us3::status::TIMEOUT:
      return US3_TIMEOUT;
    case us3::status::ERROR:
    default:
      return US3_ERROR;
  }
}

template <typename T>
us3_status_t to_capi_status(const std::pair<T, us3::status::status_t>& result) {
  return to_capi_status(result.second);
}

us3::connection_t::mode_t to_connection_mode(const us3_mode_t mode) {
  switch (mode) {
    default:
    case US3_READ:
      return us3::connection_t::READ;
    case US3_WRITE:
      return us3::connection_t::WRITE;
  }
}
}  // namespace

US3_EXTERN us3_status_t us3_open(const char* host_name,
                                 const int port,
                                 const char* path,
                                 const char* access_key,
                                 const char* secret_key,
                                 const us3_mode_t mode,
                                 const us3_microseconds_t timeout,
                                 us3_handle_t* handle) {
  // Sanity check arguments.
  if (host_name == NULL) {
    return US3_INVALID_ARGUMENT;
  }
  if (path == NULL) {
    return US3_INVALID_ARGUMENT;
  }
  if (access_key == NULL) {
    return US3_INVALID_ARGUMENT;
  }
  if (secret_key == NULL) {
    return US3_INVALID_ARGUMENT;
  }
  if (mode != US3_READ && mode != US3_WRITE) {
    return US3_INVALID_ARGUMENT;
  }
  if (handle == NULL) {
    return US3_INVALID_ARGUMENT;
  }

  // Open the connection.
  us3_handle_struct_t* new_handle = new us3_handle_struct_t;
  const us3::status::status_t result =
      new_handle->connection.open(host_name,
                                  port,
                                  path,
                                  access_key,
                                  secret_key,
                                  to_connection_mode(mode),
                                  static_cast<us3::net::timeout_t>(timeout));
  if (!us3::is_success(result)) {
    delete new_handle;
    return to_capi_status(result);
  }

  *handle = new_handle;
  return US3_SUCCESS;
}

US3_EXTERN us3_status_t us3_open_url(const char* url,
                                     const char* access_key,
                                     const char* secret_key,
                                     const us3_mode_t mode,
                                     const us3_microseconds_t timeout,
                                     us3_handle_t* handle) {
  // Sanity check arguments.
  if (url == NULL) {
    return US3_INVALID_ARGUMENT;
  }

  // Parse the URL.
  const std::pair<us3::url_parts_t, us3::status::status_t> result = us3::parse_url(url);
  if (!us3::is_success(result)) {
    return to_capi_status(result);
  }

  const us3::url_parts_t& url_parts = us3::value(result);

  // Make sure that the request was for an http URL (we don't support anything else a.t.m).
  if (url_parts.scheme != "http") {
    return US3_INVALID_URL;
  }

  // Call the main us3_open() function.
  return us3_open(url_parts.host.c_str(),
                  url_parts.port,
                  url_parts.path.c_str(),
                  access_key,
                  secret_key,
                  mode,
                  timeout,
                  handle);
}

US3_EXTERN us3_status_t us3_close(us3_handle_t handle) {
  // Sanity check arguments.
  if (!is_valid_handle(handle)) {
    return US3_INVALID_HANDLE;
  }

  // Close and delete the connection.
  us3::status::status_t result = handle->connection.close();
  delete handle;
  return to_capi_status(result);
}

US3_EXTERN us3_status_t us3_read(us3_handle_t handle,
                                 void* buf,
                                 const size_t count,
                                 const us3_microseconds_t timeout,
                                 size_t* actual_count) {
  // Sanity check arguments.
  if (!is_valid_handle(handle)) {
    return US3_INVALID_HANDLE;
  }
  if (buf == NULL) {
    return US3_INVALID_ARGUMENT;
  }
  if (actual_count == NULL) {
    return US3_INVALID_ARGUMENT;
  }

  return to_capi_status(handle->connection.read(
      buf, count, static_cast<us3::net::timeout_t>(timeout), *actual_count));
}

US3_EXTERN us3_status_t us3_write(us3_handle_t handle,
                                  const void* buf,
                                  const size_t count,
                                  const us3_microseconds_t timeout,
                                  size_t* actual_count) {
  // Sanity check arguments.
  if (!is_valid_handle(handle)) {
    return US3_INVALID_HANDLE;
  }
  if (buf == NULL) {
    return US3_INVALID_ARGUMENT;
  }
  if (actual_count == NULL) {
    return US3_INVALID_ARGUMENT;
  }

  return to_capi_status(handle->connection.write(
      buf, count, static_cast<us3::net::timeout_t>(timeout), *actual_count));
}
