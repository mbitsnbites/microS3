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

set(US3_PLATFORM_LIBS)

# Select HMAC-SHA1 implementation.
set(US3_HMAC_SHA1_SRC)
if(US3_ENABLE_SYSTEM_CRYPTO)
  if(WIN32 OR MINGW)
    set(US3_HMAC_SHA1_SRC hmac_sha1_win32.cpp)
    list(APPEND US3_PLATFORM_LIBS userenv)
  elseif(APPLE)
    set(US3_HMAC_SHA1_SRC hmac_sha1_macos.cpp)
  else()
    find_package(OpenSSL)
    if(OPENSSL_FOUND)
      set(US3_HMAC_SHA1_SRC hmac_sha1_openssl.cpp)
      list(APPEND US3_PLATFORM_LIBS ${OPENSSL_CRYPTO_LIBRARIES})
    else()
      message("Note: To use OpenSSL, install the appropriate development files.\n"
              "      E.g. on Ubuntu/Debian install libssl-dev.")
    endif()
  endif()
endif()

# If no system HMAC-SHA1 implementation was found, fall back to a custom implementation.
if(NOT US3_HMAC_SHA1_SRC)
  message("Note: Using a custom HMAC-SHA1 implementation.")
  set(US3_HMAC_SHA1_SRC hmac_sha1_custom.cpp)
endif()

# Select socket implementation.
if(WIN32 OR MINGW)
  set(US3_NETWORK_SOCKET_SRC network_socket_win32.cpp)
  list(APPEND US3_PLATFORM_LIBS ws2_32)
else()
  set(US3_NETWORK_SOCKET_SRC network_socket_posix.cpp)
endif()

# Create the library target.
set(US3_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
add_library(us3
  capi.cpp
  capi_status.cpp
  connection.cpp
  connection.hpp
  ${US3_HMAC_SHA1_SRC}
  hmac_sha1.hpp
  ${US3_NETWORK_SOCKET_SRC}
  network_socket.hpp
  return_value.hpp
  url_parser.cpp
  url_parser.hpp)
target_link_libraries(us3 PRIVATE ${US3_PLATFORM_LIBS})
target_include_directories(us3 PUBLIC ${US3_INCLUDE_DIR})
target_compile_definitions(us3 PRIVATE US3_BUILDING_LIBRARY)
set_target_properties(us3 PROPERTIES C_VISIBILITY_PRESET hidden)
set_target_properties(us3 PROPERTIES CXX_VISIBILITY_PRESET hidden)

# Are we building a shared library?
get_target_property(_us3_library_type us3 TYPE)
if(_us3_library_type STREQUAL "SHARED_LIBRARY")
  target_compile_definitions(us3 PUBLIC US3_SHARED_LIB)
endif()

# Enable compiler warnings.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(us3 PRIVATE -Wall -Wextra -Wconversion -Wsign-conversion -pedantic -Werror)
elseif(MSVC)
  target_compile_options(us3 PRIVATE /WX)
endif()

# Unit tests.
if(US3_ENABLE_TESTS)
  add_executable(hmac_sha1_test
    hmac_sha1_test.cpp
    ${US3_HMAC_SHA1_SRC})
  target_link_libraries(hmac_sha1_test doctest ${US3_PLATFORM_LIBS})
  add_test(hmac_sha1_test hmac_sha1_test)

  add_executable(url_parser_test
    url_parser_test.cpp
    url_parser.cpp)
  target_link_libraries(url_parser_test doctest)
  add_test(url_parser_test url_parser_test)
endif()

# Installation components.
install(
  TARGETS us3
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(
  DIRECTORY ${US3_INCLUDE_DIR}/us3
  DESTINATION include)
