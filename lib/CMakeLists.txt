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

# We build everything against the C++03 standard (since CMake does not support 3 as a standard, we
# use 98 which is close enough and for most purposes identical).
set(CMAKE_CXX_STANDARD 98)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(US3_SOURCES
  capi.cpp
  capi_status.cpp
  connection.cpp
  connection.hpp
  hmac_sha1.hpp
  network_socket.hpp
  return_value.hpp
  url_parser.cpp
  url_parser.hpp
  )

# Platform support.
set(US3_LIBS)
if(WIN32 OR MINGW)
  list(APPEND US3_SOURCES
    hmac_sha1_win32.cpp
    network_socket_win32.cpp)
  list(APPEND US3_LIBS
    userenv
    ws2_32)
elseif(APPLE)
  list(APPEND US3_SOURCES
    hmac_sha1_macos.cpp
    network_socket_posix.cpp)
else()
  list(APPEND US3_SOURCES
    network_socket_posix.cpp)

  # We use OpenSSL if it's present, otherwise we fall back to a custom implementation.
  find_package(OpenSSL)
  if(OPENSSL_FOUND)
    list(APPEND US3_SOURCES
      hmac_sha1_openssl.cpp)
    list(APPEND US3_LIBS
      ${OPENSSL_CRYPTO_LIBRARIES})
  else()
    message("Note: Using a custom HMAC-SHA1 implementation.\n"
            "      To use OpenSSL, install the appropriate development files.\n"
            "      E.g. on Ubuntu/Debian install libssl-dev.")
    list(APPEND US3_SOURCES
      hmac_sha1_custom.cpp)
  endif()
endif()

set(US3_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

add_library(us3 ${US3_SOURCES})
target_link_libraries(us3 PRIVATE ${US3_LIBS})
target_include_directories(us3 PUBLIC ${US3_INCLUDE_DIR})

install(
  TARGETS us3
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(
  DIRECTORY ${US3_INCLUDE_DIR}/us3
  DESTINATION include)
