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

#include "return_value.hpp"
#include <doctest.h>

// Workaround for macOS build errors.
// See: https://github.com/onqtam/doctest/issues/126
#include <iostream>

TEST_CASE("Parse valid URL strings") {
  SUBCASE("Simple HTTP URL") {
    // GIVEN
    const char* url = "http://myhost/hello/world";

    // WHEN
    const us3::result_t<us3::url_parts_t> result = us3::parse_url(url);

    // THEN
    CHECK_EQ(result.is_success(), true);
    CHECK_EQ(result->scheme, "http");
    CHECK_EQ(result->host, "myhost");
    CHECK_EQ(result->port, 80);
    CHECK_EQ(result->path, "/hello/world");
  }

  SUBCASE("Simple HTTPS URL") {
    // GIVEN
    const char* url = "https://myhost/hello.foo";

    // WHEN
    const us3::result_t<us3::url_parts_t> result = us3::parse_url(url);

    // THEN
    CHECK_EQ(result.is_success(), true);
    CHECK_EQ(result->scheme, "https");
    CHECK_EQ(result->host, "myhost");
    CHECK_EQ(result->port, 443);
    CHECK_EQ(result->path, "/hello.foo");
  }

  SUBCASE("URL with port and query") {
    // GIVEN
    const char* url = "foo://othermachine:9876/hello/world&who=me#flag";

    // WHEN
    const us3::result_t<us3::url_parts_t> result = us3::parse_url(url);

    // THEN
    CHECK_EQ(result.is_success(), true);
    CHECK_EQ(result->scheme, "foo");
    CHECK_EQ(result->host, "othermachine");
    CHECK_EQ(result->port, 9876);
    CHECK_EQ(result->path, "/hello/world&who=me#flag");
  }
}
