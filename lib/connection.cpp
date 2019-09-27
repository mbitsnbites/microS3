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

#include "hmac_sha1.hpp"
#include <algorithm>
#include <cctype>
#include <clocale>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sstream>

namespace us3 {

namespace {

struct header_field_t {
  header_field_t(const std::string& n, const std::string& v) : name(n), value(v) {
  }
  header_field_t() {
  }

  operator bool() const {
    return !name.empty();
  }

  std::string name;
  std::string value;
};

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

std::string extract_line(const char* buf, const size_t buf_size, const bool has_cr = false) {
  // Empty buffer -> empty string.
  if (buf_size == 0) {
    return std::string();
  }

  // Do we already have a CR from a previous string?
  if (has_cr && buf[0] == '\n') {
    return std::string(buf, 1);
  }

  // Try to find a new-line marker (CRLF).
  for (size_t i = 0; i < (buf_size - 1); ++i) {
    if (buf[i] == '\r' && buf[i + 1] == '\n') {
      return std::string(buf, i + 2);
    }
  }

  // Return an incomplete string.
  return std::string(buf, buf_size);
}

header_field_t parse_header_field(const std::string& line) {
  // Find the separating colon.
  const std::string::size_type colon_pos = line.find(':');
  if (colon_pos == std::string::npos) {
    return header_field_t();
  }

  // Extract the field name and turn it into lowercase.
  std::string name(line.c_str(), colon_pos);
  for (std::string::iterator it = name.begin(); it != name.end(); it++) {
    unsigned char c = static_cast<unsigned char>(*it);
    if (c >= static_cast<unsigned char>('A') && c <= static_cast<unsigned char>('Z')) {
      *it = *it - ('Z' - 'z');
    }
  }

  // Extract the field value and strip leading and trailing spaces.
  std::string value;
  {
    size_t start_pos;
    for (start_pos = colon_pos + 1; start_pos < line.size() && std::isspace(line[start_pos]);
         ++start_pos)
      ;
    if (start_pos < line.size()) {
      size_t end_pos;
      for (end_pos = line.size() - 1; end_pos > start_pos && std::isspace(line[end_pos]); --end_pos)
        ;
      value = line.substr(start_pos, end_pos - start_pos + 1);
    }
  }

  return header_field_t(name, value);
}

}  // namespace

status_t connection_t::open(const char* host_name,
                            const int port,
                            const char* path,
                            const char* access_key,
                            const char* secret_key,
                            const mode_t mode,
                            const size_t size,
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

  // Send the HTTP headers.
  const status_t headers_result = send_http_headers(host_name, path, access_key, secret_key, size);
  if (headers_result.is_error()) {
    return headers_result;
  }

  // If we're done sending data (i.e. we're in READ mode), read the HTTP response now. Otherwise
  // we defer the read to after we're done sending our message.
  m_have_http_response = false;
  if (m_mode == READ) {
    return read_http_response();
  }
  return make_result(status_t::SUCCESS);
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

  // TODO(m): Implement support for chunked transfer.
  if (m_is_chunked || !m_has_content_length) {
    return make_result<size_t>(0, status_t::UNSUPPORTED);
  }

  char* target = reinterpret_cast<char*>(buf);
  size_t bytes_left = std::min(count, m_content_left);
  size_t actual_count = 0;

  // If we have leftovers in the internal buffer we start by copying them.
  const size_t bytes_from_buffer = std::min(bytes_left, m_buffer_size);
  if (bytes_from_buffer > 0) {
    std::memcpy(target, &m_buffer[m_buffer_pos], bytes_from_buffer);
    m_buffer_pos += bytes_from_buffer;
    m_buffer_size -= bytes_from_buffer;
    target += bytes_from_buffer;
    bytes_left -= bytes_from_buffer;
    actual_count += bytes_from_buffer;
  }

  // Retrieve the rest of the bytes from the socket.
  status_t::status_enum_t status = status_t::SUCCESS;
  if (bytes_left > 0) {
    result_t<size_t> bytes_from_socket = net::recv(m_socket, target, bytes_left);
    actual_count += *bytes_from_socket;
    status = bytes_from_socket.status();
  }

  m_content_left -= actual_count;

  return make_result(actual_count, status);
}

result_t<size_t> connection_t::write(const void* buf, const size_t count) {
  // The connection must have been opened in write mode.
  if (m_mode != WRITE) {
    return make_result<size_t>(0, status_t::INVALID_OPERATION);
  }

  // TODO(m): Implement support for chunked transfer.
  if (m_is_chunked || !m_has_content_length) {
    return make_result<size_t>(0, status_t::UNSUPPORTED);
  }

  // We should not send more data than we have said that we will send.
  if (m_has_content_length && count > m_content_left) {
    return make_result<size_t>(0, status_t::INVALID_OPERATION);
  }

  // Send the buffer over the socket.
  result_t<size_t> actual_count = net::send(m_socket, buf, count);
  if (m_has_content_length && actual_count.is_success()) {
    m_content_left -= *actual_count;
  }

  // If we're done writing data, now is a good time to read the HTTP response.
  if (m_has_content_length && m_content_left == 0) {
    const status_t response_result = read_http_response();
    if (response_result.is_error()) {
      return make_result(*actual_count, response_result.status());
    }
  }

  return actual_count;
}

result_t<const char*> connection_t::get_status_line() {
  if (m_mode == NONE) {
    return make_result<const char*>(NULL, status_t::INVALID_OPERATION);
  }
  return make_result(m_status_line.c_str(), status_t::SUCCESS);
}

result_t<const char*> connection_t::get_response_field(const char* name) {
  if (m_mode == NONE) {
    return make_result<const char*>(NULL, status_t::INVALID_OPERATION);
  }

  // Look up the field among the response fields.
  std::map<std::string, std::string>::const_iterator field = m_response_fields.find(name);
  if (field == m_response_fields.end()) {
    return make_result<const char*>(NULL, status_t::NO_SUCH_FIELD);
  }

  return make_result(field->second.c_str(), status_t::SUCCESS);
}

result_t<size_t> connection_t::get_content_length() {
  if (m_mode == NONE) {
    return make_result<size_t>(0, status_t::INVALID_OPERATION);
  }
  if (!m_has_content_length) {
    return make_result<size_t>(0, status_t::NO_SUCH_FIELD);
  }
  return make_result(m_content_length, status_t::SUCCESS);
}

status_t connection_t::send_http_headers(const char* host_name,
                                         const char* path,
                                         const char* access_key,
                                         const char* secret_key,
                                         const size_t size) {
  if (m_mode == WRITE) {
    // Determine how to write data.
    if (size > 0) {
      m_has_content_length = true;
      m_content_length = size;
      m_content_left = size;
      m_is_chunked = false;
    } else {
      m_has_content_length = false;
      m_is_chunked = true;
    }
  } else {
    m_has_content_length = false;
    m_is_chunked = false;
  }

  // Gather information for the HTTP request.
  const std::string http_method = mode_to_http_method(m_mode);
  const std::string content_type = "application/octet-stream";
  const std::string date_formatted = get_date_rfc2616_gmt();
  const std::string relative_path = std::string(path);

  // Generate a signature based on the request info and the S3 secret key.
  const std::string string_to_sign =
      http_method + "\n\n" + content_type + "\n" + date_formatted + "\n" + relative_path;
  const result_t<hmac_sha1_t> digest = hmac_sha1(secret_key, string_to_sign.c_str());
  if (digest.is_error()) {
    return make_result(digest.status());
  }
  const std::string signature = std::string(digest->c_str());

  // Construct the HTTP request header.
  std::ostringstream http_header;
  http_header << http_method << " " << path << " HTTP/1.1";
  http_header << "\r\nHost: " << host_name;
  http_header << "\r\nContent-Type: " << content_type;
  http_header << "\r\nDate: " << date_formatted;
  http_header << "\r\nAuthorization: AWS " << access_key << ":" << signature;
  if (m_has_content_length) {
    http_header << "\r\nContent-Length: " << m_content_length;
  }
  http_header << "\r\n\r\n";

  // Send the HTTP header.
  {
    status_t header_send_status = send_string(m_socket, http_header.str());
    if (header_send_status.is_error()) {
      return make_result(header_send_status.status());
    }
  }

  return make_result(status_t::SUCCESS);
}

status_t connection_t::read_data_to_buffer() {
  // End of buffer reached?
  if (m_buffer_pos == MAX_BUFFER_SIZE) {
    m_buffer_pos = 0;
  }

  // Try to read enough data to fill the buffer.
  const size_t bytes_to_read = MAX_BUFFER_SIZE - (m_buffer_pos + m_buffer_size);
  result_t<size_t> result = net::recv(m_socket, &m_buffer[m_buffer_pos], bytes_to_read);
  if (result.is_error()) {
    return make_result(result.status());
  }
  m_buffer_size += *result;

  return make_result(status_t::SUCCESS);
}

status_t connection_t::read_http_response() {
  // We do not have to read the HTTP reponse again if we already have it.
  if (m_have_http_response) {
    return make_result(status_t::SUCCESS);
  }

  m_status_line.clear();

  if (m_mode == READ) {
    m_content_length = 0;
    m_content_left = 0;
    m_has_content_length = false;
    m_is_chunked = false;
  }

  std::string incomplete_line;
  while (!m_have_http_response) {
    // Read more data into our buffer.
    status_t result = read_data_to_buffer();
    if (result.is_error()) {
      return result;
    }

    // Read lines.
    while (m_buffer_size > 0) {
      // Extract a new string from the buffer.
      const bool has_cr =
          (incomplete_line.size() > 0 && incomplete_line[incomplete_line.size() - 1] == '\r');
      std::string line = extract_line(&m_buffer[m_buffer_pos], m_buffer_size, has_cr);
      m_buffer_pos += line.size();
      m_buffer_size -= line.size();

      // Prepend a previous incomplete line (if any).
      line = incomplete_line + line;

      // Final blank line that terminates the HTTP response?
      if (line == "\r\n") {
        m_have_http_response = true;
        break;
      }

      // Incomplete line (i.e. we've reached the end of the buffer but we don't have a terminating
      // CRLF)?
      if (m_buffer_size == 0 &&
          (line.size() < 2 || (line[line.size() - 2] != '\r' || line[line.size() - 1] != '\n'))) {
        incomplete_line = line;
        break;
      } else {
        incomplete_line = "";
      }

      // Sanity check.
      if (line.size() < 2) {
        return make_result(status_t::ERROR);
      }

      // We now have a CRLF-terminated HTTP response line.
      if (m_status_line.empty()) {
        // The first line is the status line. Remove the trailing \r\n.
        m_status_line = line.substr(0, line.size() - 2);
      } else {
        header_field_t header_field = parse_header_field(line);
        if (header_field) {
          m_response_fields[header_field.name] = header_field.value;
        }
      }
    }
  }

  if (m_mode == READ) {
    // Parse the content-length field (if present).
    {
      std::map<std::string, std::string>::const_iterator field =
          m_response_fields.find("content-length");
      if (field != m_response_fields.end()) {
        const long int x = std::strtol(field->second.c_str(), NULL, 10);
        m_content_length = static_cast<size_t>(x);
        m_content_left = m_content_length;
        m_has_content_length = true;
      }
    }

    // Check if this is a chunked transfer.
    {
      std::map<std::string, std::string>::const_iterator field =
          m_response_fields.find("transfer-encoding");
      if (field != m_response_fields.end()) {
        if (field->second.find("chunked") != std::string::npos) {
          m_is_chunked = true;
        }
      }
    }
  }

  // Check the HTTP status code (should be "HTTP/1.1 200 OK").
  if (std::strncmp(m_status_line.c_str(), "HTTP/1.1 ", 9) != 0) {
    return make_result(status_t::UNSUPPORTED);
  }
  const int status_code = (static_cast<int>(m_status_line[9] - '0') * 100) +
                          (static_cast<int>(m_status_line[10] - '0') * 10) +
                          static_cast<int>(m_status_line[11] - '0');
  if (status_code == 200) {
    return make_result(status_t::SUCCESS);
  } else if (status_code == 403) {
    return make_result(status_t::FORBIDDEN);
  } else if (status_code == 404) {
    return make_result(status_t::NOT_FOUND);
  } else {
    return make_result(status_t::ERROR);
  }
}

}  // namespace us3
