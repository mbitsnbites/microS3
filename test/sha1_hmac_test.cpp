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
#include <string>

TEST_CASE("Hash some strings") {
  SUBCASE("Hello world") {
    // GIVEN
    const char* key = "zupaS3cret!";
    const char* data = "Hello world!";

    // WHEN
    const auto result = us3::sha1_hmac(key, data);

    // THEN
    CHECK_EQ(us3::is_success(result), true);
    const auto actual = std::string(us3::value(result).c_str());
    CHECK_EQ(actual, "vfSHGKMkJ32kPV1xpaeZG74J5Fg=");
  }

  SUBCASE("Empty data") {
    // GIVEN
    const char* key = "abcdefghijklmnopqrstuvwxyz";
    const char* data = "";

    // WHEN
    const auto result = us3::sha1_hmac(key, data);

    // THEN
    CHECK_EQ(us3::is_success(result), true);
    const auto actual = std::string(us3::value(result).c_str());
    CHECK_EQ(actual, "KM+4KvZd8CLgj6GmcSEGjB1IC8g=");
  }
}
