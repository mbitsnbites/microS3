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

#include "hmac_sha1.hpp"

#include <cstring>
#include <openssl/hmac.h>

namespace us3 {

result_t<hmac_sha1_t> hmac_sha1(const char* key, const char* data) {
  unsigned char raw_digest[hmac_sha1_t::HMAC_SHA1_RAW_SIZE];
  (void)::HMAC(::EVP_sha1(),
               key,
               static_cast<int>(std::strlen(key)),
               reinterpret_cast<const unsigned char*>(data),
               std::strlen(data),
               reinterpret_cast<unsigned char*>(&raw_digest[0]),
               NULL);
  return make_result(hmac_sha1_t(raw_digest));
}

}  // namespace us3
