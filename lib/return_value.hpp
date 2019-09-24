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

#ifndef US3_RETURN_VALUE_HPP_
#define US3_RETURN_VALUE_HPP_

namespace us3 {

/// @brief A result object with a status code.
class status_t {
public:
  /// @brief Possible function result codes.
  enum status_enum_t {
    SUCCESS,            ///< No error occurred.
    ERROR,              ///< An unspecified error occurred.
    INVALID_ARGUMENT,   ///< An invalid argument was passed to a function.
    INVALID_OPERATION,  ///< An invalid operation was requested.
    INVALID_URL,        ///< An invalid URL was passed to a function.
    NO_HOST,            ///< No such host was found.
    DENIED,             ///< Access denied.
    REFUSED,            ///< The connection was refused.
    UNREACHABLE,        ///< The network is unreachable.
    CONNECTION_RESET,   ///< The connection was reset by the peer.
    TIMEOUT,            ///< The operation timed out.
    NO_SUCH_FIELD       ///< The requested field was not found.
  };

  explicit status_t(const status_enum_t s) : m_status(s) {
  }

  /// @brief Get the result status.
  status_enum_t status() const {
    return m_status;
  }

  /// @brief Check if the status indicates success.
  bool is_success() const {
    return m_status == SUCCESS;
  }

  /// @brief Check if the status indicates an error.
  bool is_error() const {
    return m_status != SUCCESS;
  }

private:
  const status_enum_t m_status;
};

/// @brief A result object with a value and a status code.
template <typename T>
class result_t : public status_t {
public:
  result_t(const T& v, const status_enum_t s) : status_t(s), m_value(v) {
  }

  /// @brief Get the result value.
  const T& operator*() const {
    return m_value;
  }

  /// @brief Get a reference to the result value.
  const T* operator->() const {
    return &m_value;
  }

private:
  const T m_value;
};

namespace {

/// @brief Construct a result object with a value.
/// @param v The result value.
/// @note The status of the result object is @c SUCCESS.
template <typename T>
result_t<T> make_result(const T& v) {
  return result_t<T>(v, status_t::SUCCESS);
}

/// @brief Construct a result object with a value and a status.
/// @param v The result value.
/// @param s The result status.
template <typename T>
result_t<T> make_result(const T& v, const status_t::status_enum_t s) {
  return result_t<T>(v, s);
}

/// @brief Construct a result object with a status.
/// @param s The result status.
status_t make_result(const status_t::status_enum_t s) {
  return status_t(s);
}

}  // namespace

}  // namespace us3

#endif  // US3_RETURN_VALUE_HPP_
