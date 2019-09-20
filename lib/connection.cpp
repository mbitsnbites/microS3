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

#include "sha1_hmac.hpp"
#include <clocale>
#include <ctime>

namespace us3 {

namespace {

std::string get_date_rfc2616_gmt() {
  // TODO(m): setlocale() is not guaranteed to be thread safe. Can we do this in a more thread safe
  // manner?

  // Set the locale to "C" (and save old locale).
  const char* old_locale = std::setlocale(LC_ALL, NULL);
  std::setlocale(LC_ALL, "C");

  // Get the current date & time.
  std::time_t now = std::time(0);
  std::tm now_gmt = *std::gmtime(&now);

  // Format the date & time according to RFC2616.
  char buf[100];
  if (std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &now_gmt) == 0) {
    buf[0] = '\0';
  }

  // Restore the old locale.
  std::setlocale(LC_ALL, old_locale);

  return std::string(&buf[0]);
}

std::string mode_to_http_method(const connection_t::mode_t mode) {
  if (mode == connection_t::WRITE) {
    return "PUT";
  } else {
    return "GET";
  }
}

status_t send_string(net::socket_t socket, const std::string& str) {
  size_t remaining = str.size();
  size_t sent = 0;
  while (remaining > 0) {
    const result_t<size_t> count = net::send(socket, &str[sent], remaining);
    if (count.is_error()) {
      return make_result(count.status());
    }
    remaining -= *count;
    sent += *count;
  }
  return make_result(status_t::SUCCESS);
}

}  // namespace

status_t connection_t::open(const char* host_name,
                            const int port,
                            const char* path,
                            const char* access_key,
                            const char* secret_key,
                            const mode_t mode,
                            const net::timeout_t connect_timeout,
                            const net::timeout_t socket_timeout) {
  // Sanity check arguments.
  // Note: All pointers are guaranteed to be non-NULL at this point.
  if (port < 1 || port > 65535) {
    return make_result(status_t::INVALID_OPERATION);
  }

  // We must not open a connection that is already opened.
  if (m_mode != NONE) {
    return make_result(status_t::INVALID_OPERATION);
  }

  // Connect to the remote host.
  result_t<net::socket_t> socket = net::connect(host_name, port, connect_timeout, socket_timeout);
  if (socket.is_error()) {
    return make_result(socket.status());
  }

  // We're now officially connected.
  m_mode = mode;
  m_socket = *socket;

  // Gather information for the HTTP request.
  const std::string http_method = mode_to_http_method(mode);
  const std::string content_type = "application/octet-stream";
  const std::string date_formatted = get_date_rfc2616_gmt();
  const std::string relative_path = std::string(path);

  // Generate a signature based on the request info and the S3 secret key.
  const std::string string_to_sign =
      http_method + "\n\n" + content_type + "\n" + date_formatted + "\n" + relative_path;
  const result_t<sha1_hmac_t> digest = sha1_hmac(secret_key, string_to_sign.c_str());
  if (digest.is_error()) {
    return make_result(digest.status());
  }
  const std::string signature = std::string(digest->c_str());

  // Construct the HTTP request header.
  std::string http_header;
  http_header += http_method + " " + std::string(path) + " HTTP/1.1";
  http_header += "\r\nHost: " + std::string(host_name);
  http_header += "\r\nContent-Type: " + content_type;
  http_header += "\r\nDate: " + date_formatted;
  http_header += "\r\nAuthorization: AWS " + std::string(access_key) + ":" + signature;
  http_header += "\r\n\r\n";

  // Send the HTTP header.
  {
    status_t header_send_status = send_string(m_socket, http_header);
    if (header_send_status.is_error()) {
      return  make_result(header_send_status.status());;
    }
  }

  // Read the HTTP response.
  // TODO(m): Implement me!
  m_content_size = 0;
  return  make_result(status_t::ERROR);
}

status_t connection_t::close() {
  // We can not close a connection that is already closed.
  if (m_mode == NONE) {
    return make_result(status_t::INVALID_OPERATION);
  }

  // Disconnect.
  status_t result = net::disconnect(m_socket);

  // We're no longer connected.
  m_mode = NONE;
  m_socket = NULL;

  return result;
}

result_t<size_t> connection_t::read(void* buf, const size_t count) {
  // The connection must have been opened in read mode.
  if (m_mode != READ) {
    return make_result<size_t>(0, status_t::INVALID_OPERATION);
  }

  return net::recv(m_socket, buf, count);
}

result_t<size_t> connection_t::write(const void* buf, const size_t count) {
  // The connection must have been opened in write mode.
  if (m_mode != WRITE) {
    return make_result<size_t>(0, status_t::INVALID_OPERATION);
  }

  return net::send(m_socket, buf, count);
}

}  // namespace us3
