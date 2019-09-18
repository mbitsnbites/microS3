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

#ifndef US3_SHA1_HMAC_HPP_
#define US3_SHA1_HMAC_HPP_

#include "return_value.hpp"
#include <string>

namespace us3 {

/// @brief A SHA1 HMAC digest.
class sha1_hmac_t {
public:
  // The raw SHA1 HMAC digest size is 20 bytes.
  static const int SHA1_HMAC_RAW_SIZE = 20;

  // The base64-encoded SHA1 HMAC digest size is 28 bytes (excluding the zero termination).
  static const int SHA1_HMAC_BASE64_SIZE = 28;

  /// @brief Construct a base64 encoded digest from a raw digest.
  sha1_hmac_t(const unsigned char* raw_digest) {
    static const char* const BASE64_CHARS =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (size_t i = 0, j = 0; i < SHA1_HMAC_RAW_SIZE; i += 3, j += 4) {
      // Collect three bytes (24 bits) from the input buffer.
      unsigned long v = static_cast<unsigned long>(raw_digest[i]) << 16;
      if (i + 1 < SHA1_HMAC_RAW_SIZE) {
        v = v | (static_cast<unsigned long>(raw_digest[i + 1]) << 8);
      }
      if (i + 2 < SHA1_HMAC_RAW_SIZE) {
        v = v | static_cast<unsigned long>(raw_digest[i + 2]);
      }

      // Produce four bytes for the output buffer.
      m_digest[j] = BASE64_CHARS[(v >> 18) & 0x3F];
      m_digest[j + 1] = BASE64_CHARS[(v >> 12) & 0x3F];
      if (i + 1 < SHA1_HMAC_RAW_SIZE) {
        m_digest[j + 2] = BASE64_CHARS[(v >> 6) & 0x3F];
      } else {
        m_digest[j + 2] = '=';
      }
      if (i + 2 < SHA1_HMAC_RAW_SIZE) {
        m_digest[j + 3] = BASE64_CHARS[v & 0x3F];
      } else {
        m_digest[j + 3] = '=';
      }
    }

    // Zero-terminate.
    m_digest[SHA1_HMAC_BASE64_SIZE] = '\0';
  }

  /// @brief Get the base64 encoded digest as a C string.
  const char* c_str() const {
    return &m_digest[0];
  }

private:
  char m_digest[SHA1_HMAC_BASE64_SIZE + 1];
};

/// @brief Generate the SHA1 HMAC hash for a string.
/// @param key The secret key.
/// @param data The data to hash.
/// @returns the digest.
std::pair<sha1_hmac_t, status::status_t> sha1_hmac(const std::string& key, const std::string& data);

}  // namespace us3

#endif  // US3_SHA1_HMAC_HPP_
