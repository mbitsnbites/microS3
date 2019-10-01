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
#include <cstddef>
#include <map>
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

  connection_t()
      : m_mode(NONE),
        m_socket(NULL),
        m_buffer_pos(0),
        m_buffer_size(0),
        m_have_http_response(false),
        m_content_length(0),
        m_content_left(0),
        m_has_content_length(false),
        m_is_chunked(false) {
  }

  ~connection_t() {
    if (m_mode != NONE) {
      close();
    }
  }

  /**
   * @brief Open the connection.
   *
   * This method opens a connection to the specified host and initiates S3 authentication by sending
   * the apropriate HTTP message headers. If this is a READ request, the HTTP response is also read.
   *
   * @param host_name Name of the host.
   * @param port Port to connection to.
   * @param path Full path to the object (including the leading slash).
   * @param access_key The S3 access key.
   * @param secret_key The S3 secret key.
   * @param mode Stream mode.
   * @param size Number of bytes to send (ignored for READ connections).
   * @param connect_timeout Connection timeout in μs, or 0 for no timeout.
   * @param socket_timeout Socket timeout in μs, or 0 for no timeout
   * @returns status_t::SUCCESS for success, otherwise an error code.
   */
  status_t open(const char* host_name,
                const int port,
                const char* path,
                const char* access_key,
                const char* secret_key,
                const mode_t mode,
                const size_t size,
                const net::timeout_t connect_timeout,
                const net::timeout_t socket_timeout);

  /**
   * @brief Close the connection.
   * @returns status_t::SUCCESS for success, otherwise an error code.
   */
  status_t close();

  /**
   * @brief Read data from the stream.
   * @param buf The buffer to read to.
   * @param count The number of bytes to read.
   * @returns the actual number of bytes read. The actual count may be less than @c count. If the
   * return value is zero, the end of the stream was reached.
   */
  result_t<size_t> read(void* buf, const size_t count);

  /**
   * @brief Write data to the stream.
   * @param buf The buffer to write from.
   * @param count The number of bytes to write.
   * @returns the actual number of bytes written. The actual count may be less than @c count.
   */
  result_t<size_t> write(const void* buf, const size_t count);

  /**
   * @brief Get the status line from the HTTP response.
   * @note The HTTP response must have been received before using this function.
   */
  result_t<const char*> get_status_line();

  /**
   * @brief Get a HTTP response field.
   * @param name Name of the field (must be lower case).
   * @returns the value of the response field, or status_t::NO_SUCH_FIELD if the field was not part
   * of the response.
   * @note The HTTP response must have been received before using this function.
   */
  result_t<const char*> get_response_field(const char* name);

  /**
   * @brief Get the content length of the HTTP message.
   * @returns the size of the message content, in bytes.
   * @note The HTTP response must have been received before using this function.
   */
  result_t<size_t> get_content_length();

private:
  static const size_t MAX_BUFFER_SIZE = 1024;

  status_t send_http_headers(const char* host_name,
                             const char* path,
                             const char* access_key,
                             const char* secret_key,
                             const size_t size);
  status_t read_data_to_buffer();
  status_t read_http_response();

  mode_t m_mode;
  net::socket_t m_socket;

  // Internal buffer used for reading the HTTP response.
  size_t m_buffer_pos;
  size_t m_buffer_size;
  char m_buffer[MAX_BUFFER_SIZE];

  // HTTP response values.
  bool m_have_http_response;
  std::string m_status_line;
  std::map<std::string, std::string> m_response_fields;
  size_t m_content_length;
  size_t m_content_left;
  bool m_has_content_length;
  bool m_is_chunked;
};

}  // namespace us3

#endif  // US3_CONNECTION_HPP_
