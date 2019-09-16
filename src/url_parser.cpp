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

#include "url_parser.hpp"
#include <cstdlib>

namespace us3 {
namespace {
int string_to_int(const char* str) {
  char* end;
  const long int x = std::strtol(str, &end, 10);
  if (x == 0L) {
    return -1;
  }
  return static_cast<int>(x);
}

} // namespace

std::pair<url_parts_t, status::status_t> parse_url(const char* url) {
  url_parts_t parts;
  int k = 0;
  int part_start = 0;

  // Extract the scheme.
  for (k = part_start; (url[k] != 0) && (url[k] != ':'); ++k);
  if ((url[k] != ':') || (url[k + 1] != '/') || (url[k + 2] != '/')) {
    return std::make_pair(parts, status::INVALID_URL);
  }
  parts.scheme = std::string(&url[part_start], k - part_start);
  part_start = k + 3;

  // Extract the host.
  for (k = part_start; (url[k] != 0) && (url[k] != '@') && (url[k] != ':') && (url[k] != '/'); ++k);
  if ((url[k] == 0) || (url[k] == '@')) {
    return std::make_pair(parts, status::INVALID_URL);
  }
  const bool has_port = (url[k] == ':');
  parts.host = std::string(&url[part_start], k - part_start);
  part_start = k + 1;

  // Extract the port.
  if (has_port) {
    for (k = part_start; (url[k] != 0) && (url[k] != '/'); ++k);
    if (url[k] == '/') {
      return std::make_pair(parts, status::INVALID_URL);
    }
    char* int_end;
    parts.port = static_cast<int>(std::strtol(&url[part_start], &int_end, 10));
    if ((parts.port == 0) || (int_end != &url[k])) {
      return std::make_pair(parts, status::INVALID_URL);
    }
    part_start = k + 1;
  } else {
    // Default to a port based on the scheme.
    parts.port = (parts.scheme == "https" ? 443 : 80);
  }

  // The rest is the path (we include query & fragment in the path.
  parts.path = std::string(&url[part_start]);

  return std::make_pair(parts, status::SUCCESS);
}
}  // namespace us3
