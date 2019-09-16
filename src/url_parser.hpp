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

#ifndef US3_URL_PARSER_HPP_
#define US3_URL_PARSER_HPP_

#include "return_value.hpp"
#include <string>

namespace us3 {
struct url_parts_t {
  std::string scheme;
  std::string host;
  std::string path;
  int port;
};

/// @brief Parse the given URL.
/// @param url The URL to parse.
/// @returns the different parts of the URL in a @c url_parts_t struct.
std::pair<url_parts_t, status::status_t> parse_url(const char* url);

}  // namespace us3

#endif  // US3_URL_PARSER_HPP_
