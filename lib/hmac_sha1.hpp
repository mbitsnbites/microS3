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

#ifndef US3_HMAC_SHA1_HPP_
#define US3_HMAC_SHA1_HPP_

#include "return_value.hpp"

namespace us3 {

/// @brief An HMAC-SHA1 digest.
class hmac_sha1_t {
public:
  // The raw HMAC-SHA1 digest size is 20 bytes.
  static const int HMAC_SHA1_RAW_SIZE = 20;

  /// @brief Construct a base64 encoded digest from a raw digest.
  /// @param raw_digest The raw digest buffer.
  /// @note This implementation is hardcoded for converting 20 raw bytes to 28 base64 chars.
  hmac_sha1_t(const unsigned char (&raw_digest)[20]) {
    static const char* const BASE64_CHARS =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    // Loop over all input bytes and emit encoded bytes.
    unsigned long v = 0;
    int i = 0, j = 0;
    for (; i < 20; ++i) {
      v = (v << 8) | static_cast<unsigned long>(raw_digest[i]);

      // Emit four encoded chars for every three input bytes.
      if ((i % 3) == 2) {
        m_digest[j++] = BASE64_CHARS[(v >> 18) & 0x3F];
        m_digest[j++] = BASE64_CHARS[(v >> 12) & 0x3F];
        m_digest[j++] = BASE64_CHARS[(v >> 6) & 0x3F];
        m_digest[j++] = BASE64_CHARS[v & 0x3F];
        v = 0;
      }
    }

    // Since 20/3 = 6+2/3, we have two source bytes left (emit as three encoded chars).
    m_digest[j++] = BASE64_CHARS[(v >> 10) & 0x3F];
    m_digest[j++] = BASE64_CHARS[(v >> 4) & 0x3F];
    m_digest[j++] = BASE64_CHARS[(v << 2) & 0x3F];
    m_digest[j++] = '=';
    m_digest[j++] = '\0';
  }

  /// @brief Get the base64 encoded digest as a C string.
  const char* c_str() const {
    return &m_digest[0];
  }

private:
  // The base64-encoded HMAC-SHA1 digest size is 28 bytes (excluding the zero termination).
  static const int HMAC_SHA1_BASE64_SIZE = 28;

  char m_digest[HMAC_SHA1_BASE64_SIZE + 1];
};

/// @brief Generate the HMAC-SHA1 hash for a string.
/// @param key The secret key.
/// @param data The data to hash.
/// @returns the digest.
result_t<hmac_sha1_t> hmac_sha1(const char* key, const char* data);

}  // namespace us3

#endif  // US3_HMAC_SHA1_HPP_
