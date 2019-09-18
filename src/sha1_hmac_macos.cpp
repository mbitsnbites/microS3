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

#include "sha1_hmac.hpp"
#include <CommonCrypto/CommonHMAC.h>
#include <cstring>

namespace us3 {

std::pair<sha1_hmac_t, status::status_t> sha1_hmac(const char* key, const char* data) {
  unsigned char raw_digest[sha1_hmac_t::SHA1_HMAC_RAW_SIZE];
  CCHmac(kCCHmacAlgSHA1, key, std::strlen(key), data, std::strlen(data), &raw_digest[0]);
  return std::make_pair(sha1_hmac_t(&raw_digest[0]), status::SUCCESS);
}

}  // namespace us3
