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

#include <doctest.h>
#include <return_value.hpp>
#include <sha1_hmac.hpp>

namespace {

std::string to_hex(const std::string& digest) {
  static const char HEX_CHARS[] = "0123456789abcdef";
  std::string result;
  for (const auto x : digest) {
    const auto byte = static_cast<unsigned int>(x);
    result += HEX_CHARS[(byte >> 4) & 15];
    result += HEX_CHARS[byte & 15];
  }
  return result;
}

}  // namespace

TEST_CASE("Hash some strings") {
  SUBCASE("Hello world") {
    // GIVEN
    const std::string key = "zupaS3cret!";
    const std::string data = "Hello world!";

    // WHEN
    const auto result = us3::sha1_hmac(key, data);

    // THEN
    CHECK_EQ(us3::is_success(result), true);
    const auto actual = to_hex(us3::value(result));
    CHECK_EQ(actual, "bdf48718a324277da43d5d71a5a7991bbe09e458");
  }

  SUBCASE("Empty data") {
    // GIVEN
    const std::string key = "abcdefghijklmnopqrstuvwxyz";
    const std::string data = "";

    // WHEN
    const auto result = us3::sha1_hmac(key, data);

    // THEN
    CHECK_EQ(us3::is_success(result), true);
    const auto actual = to_hex(us3::value(result));
    CHECK_EQ(actual, "28cfb82af65df022e08fa1a67121068c1d480bc8");
  }
}
