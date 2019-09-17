###################################################################################################
# Copyright (c) 2019 Marcus Geelnard
#
# This software is provided 'as-is', without any express or implied warranty. In no event will the
# authors be held liable for any damages arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose, including commercial
# applications, and to alter it and redistribute it freely, subject to the following restrictions:
#
#  1. The origin of this software must not be misrepresented; you must not claim that you wrote
#     the original software. If you use this software in a product, an acknowledgment in the
#     product documentation would be appreciated but is not required.
#
#  2. Altered source versions must be plainly marked as such, and must not be misrepresented as
#     being the original software.
#
#  3. This notice may not be removed or altered from any source distribution.
###################################################################################################

# Tests use C++11.
set(CMAKE_CXX_STANDARD 11)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# We use doctest for tests.
# See: https://github.com/onqtam/doctest/
add_library(doctest
            doctest/doctest_main.cpp
            doctest/doctest.h)
target_include_directories(doctest PUBLIC doctest)

# Unit tests.
include(us3_add_test.cmake)
us3_add_test(sha1_hmac_test.cpp)
us3_add_test(url_parser_test.cpp)
