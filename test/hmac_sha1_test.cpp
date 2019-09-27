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
#include <hmac_sha1.hpp>
#include <return_value.hpp>
#include <string>

// Workaround for macOS build errors.
// See: https://github.com/onqtam/doctest/issues/126
#include <iostream>

TEST_CASE("Hash some strings") {
  SUBCASE("Hello world") {
    // GIVEN
    const char* key = "zupaS3cret!";
    const char* data = "Hello world!";

    // WHEN
    const auto result = us3::hmac_sha1(key, data);

    // THEN
    CHECK_EQ(result.is_success(), true);
    CHECK_EQ(std::string(result->c_str()), "vfSHGKMkJ32kPV1xpaeZG74J5Fg=");
  }

  SUBCASE("Empty data") {
    // GIVEN
    const char* key = "abcdefghijklmnopqrstuvwxyz";
    const char* data = "";

    // WHEN
    const auto result = us3::hmac_sha1(key, data);

    // THEN
    CHECK_EQ(result.is_success(), true);
    CHECK_EQ(std::string(result->c_str()), "KM+4KvZd8CLgj6GmcSEGjB1IC8g=");
  }

  SUBCASE("Short key, long data") {
    // GIVEN
    const char* key = "123";
    const char* data =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt "
        "ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation "
        "ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in "
        "reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur "
        "sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id "
        "est laborum.";

    // WHEN
    const auto result = us3::hmac_sha1(key, data);

    // THEN
    CHECK_EQ(result.is_success(), true);
    CHECK_EQ(std::string(result->c_str()), "SNwEVRL0T9MlxLU2mD1DonYxSt0=");
  }

  SUBCASE("Long key, short data") {
    // GIVEN
    const char* key =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt "
        "ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation "
        "ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in "
        "reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur "
        "sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id "
        "est laborum.";
    const char* data = "123";

    // WHEN
    const auto result = us3::hmac_sha1(key, data);

    // THEN
    CHECK_EQ(result.is_success(), true);
    CHECK_EQ(std::string(result->c_str()), "n2BDD6BL0i3/OUo+xgTNQNL5zv0=");
  }
}
