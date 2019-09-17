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

#include <utility>

namespace us3 {
namespace status {
/// @brief Possible function result codes.
enum status_t {
  SUCCESS,            ///< No error occurred.
  ERROR,              ///< An unspcified error occurred.
  INVALID_ARGUMENT,   ///< An invalid argument was passed to a function.
  INVALID_OPERATION,  ///< An invalid operation was requested.
  INVALID_URL,        ///< An invalid URL was passed to a function.
  TIMEOUT             ///< The operation timed out.
};
}  // namespace status

namespace {
/// @brief Get the value part of a function result.
/// @param result The function result.
/// @returns the return value of the function result.
template <typename T>
T& value(std::pair<T, status::status_t>& result) {
  return result.first;
}

/// @brief Get the value part of a function result.
/// @param result The function result.
/// @returns the return value of the function result.
template <typename T>
const T& value(const std::pair<T, status::status_t>& result) {
  return result.first;
}

/// @brief Determine if a status code indicates success or error.
/// @param code The status code.
/// @returns true if the code indicates that there was no error.
bool is_success(const status::status_t result) {
  return result == status::SUCCESS;
}

/// @brief Determine if a function result indicates success or error.
/// @param result The function result.
/// @returns true if the result indicates that there was no error.
template <typename T>
bool is_success(const std::pair<T, status::status_t>& result) {
  return is_success(result.second);
}
}  // namespace

}  // namespace us3

#endif  // US3_RETURN_VALUE_HPP_
