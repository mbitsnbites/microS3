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

#include <us3/us3.h>

#include "connection.hpp"
#include "network_socket.hpp"
#include "return_value.hpp"
#include "url_parser.hpp"
#include <cstring>

struct us3_handle_struct_t {
  us3::connection_t connection;
};

namespace {
bool is_valid_handle(const us3_handle_t& handle) {
  // TODO(m): Do a more robust check.
  return handle != NULL;
}

us3_status_t to_capi_status(const us3::status_t& result) {
  switch (result.status()) {
    case us3::status_t::SUCCESS:
      return US3_SUCCESS;
    case us3::status_t::INVALID_ARGUMENT:
      return US3_INVALID_ARGUMENT;
    case us3::status_t::INVALID_OPERATION:
      return US3_INVALID_OPERATION;
    case us3::status_t::INVALID_URL:
      return US3_INVALID_URL;
    case us3::status_t::NO_HOST:
      return US3_NO_HOST;
    case us3::status_t::DENIED:
      return US3_DENIED;
    case us3::status_t::REFUSED:
      return US3_REFUSED;
    case us3::status_t::UNREACHABLE:
      return US3_UNREACHABLE;
    case us3::status_t::CONNECTION_RESET:
      return US3_CONNECTION_RESET;
    case us3::status_t::TIMEOUT:
      return US3_TIMEOUT;
    case us3::status_t::UNSUPPORTED:
      return US3_UNSUPPORTED;
    case us3::status_t::NO_SUCH_FIELD:
      return US3_NO_SUCH_FIELD;
    case us3::status_t::ERROR:
    default:
      return US3_ERROR;
  }
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
                                 const us3_microseconds_t connect_timeout,
                                 const us3_microseconds_t socket_timeout,
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
  const us3::status_t result =
      new_handle->connection.open(host_name,
                                  port,
                                  path,
                                  access_key,
                                  secret_key,
                                  to_connection_mode(mode),
                                  static_cast<us3::net::timeout_t>(connect_timeout),
                                  static_cast<us3::net::timeout_t>(socket_timeout));
  if (result.is_error()) {
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
                                     const us3_microseconds_t connect_timeout,
                                     const us3_microseconds_t socket_timeout,
                                     us3_handle_t* handle) {
  // Sanity check arguments.
  if (url == NULL) {
    return US3_INVALID_ARGUMENT;
  }

  // Parse the URL.
  const us3::result_t<us3::url_parts_t> url_parts = us3::parse_url(url);
  if (url_parts.is_error()) {
    return to_capi_status(url_parts);
  }

  // Make sure that the request was for an http URL (we don't support anything else a.t.m).
  if (url_parts->scheme != "http") {
    return US3_INVALID_URL;
  }

  // Call the main us3_open() function.
  return us3_open(url_parts->host.c_str(),
                  url_parts->port,
                  url_parts->path.c_str(),
                  access_key,
                  secret_key,
                  mode,
                  connect_timeout,
                  socket_timeout,
                  handle);
}

US3_EXTERN us3_status_t us3_close(us3_handle_t handle) {
  // Sanity check arguments.
  if (!is_valid_handle(handle)) {
    return US3_INVALID_HANDLE;
  }

  // Close and delete the connection.
  us3::status_t result = handle->connection.close();
  delete handle;
  return to_capi_status(result);
}

US3_EXTERN us3_status_t us3_read(us3_handle_t handle,
                                 void* buf,
                                 const size_t count,
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

  us3::result_t<size_t> result = handle->connection.read(buf, count);
  *actual_count = *result;
  return to_capi_status(result);
}

US3_EXTERN us3_status_t us3_write(us3_handle_t handle,
                                  const void* buf,
                                  const size_t count,
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

  us3::result_t<size_t> result = handle->connection.write(buf, count);
  *actual_count = *result;
  return to_capi_status(result);
}

US3_EXTERN us3_status_t us3_get_status_line(us3_handle_t handle, const char** status_line) {
  // Sanity check arguments.
  if (!is_valid_handle(handle)) {
    return US3_INVALID_HANDLE;
  }
  if (status_line == NULL) {
    return US3_INVALID_ARGUMENT;
  }

  us3::result_t<const char*> result = handle->connection.get_status_line();
  *status_line = *result;
  return to_capi_status(result);
}

US3_EXTERN us3_status_t us3_get_response_field(us3_handle_t handle,
                                               const char* name,
                                               const char** value) {
  // Sanity check arguments.
  if (!is_valid_handle(handle)) {
    return US3_INVALID_HANDLE;
  }
  if (name == NULL) {
    return US3_INVALID_ARGUMENT;
  }
  if (value == NULL) {
    return US3_INVALID_ARGUMENT;
  }

  us3::result_t<const char*> result = handle->connection.get_response_field(name);
  *value = *result;
  return to_capi_status(result);
}

US3_EXTERN us3_status_t us3_get_content_length(us3_handle_t handle, size_t* content_length) {
  // Sanity check arguments.
  if (!is_valid_handle(handle)) {
    return US3_INVALID_HANDLE;
  }
  if (content_length == NULL) {
    return US3_INVALID_ARGUMENT;
  }

  us3::result_t<size_t> result = handle->connection.get_content_length();
  *content_length = *result;
  return to_capi_status(result);
}
