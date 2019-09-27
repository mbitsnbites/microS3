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
#include <stdint.h>
#include <vector>

namespace us3 {

namespace {

// Read a big endian 32-bit word from a byte array.
uint32_t get_uint32_be(const unsigned char* ptr) {
  return (static_cast<uint32_t>(ptr[0]) << 24) | (static_cast<uint32_t>(ptr[1]) << 16) |
         (static_cast<uint32_t>(ptr[2]) << 8) | static_cast<uint32_t>(ptr[3]);
}

// Write a big endian 32-bit word to a byte array.
void set_uint32_be(const uint32_t x, unsigned char* ptr) {
  ptr[0] = static_cast<unsigned char>(x >> 24);
  ptr[1] = static_cast<unsigned char>(x >> 16);
  ptr[2] = static_cast<unsigned char>(x >> 8);
  ptr[3] = static_cast<unsigned char>(x);
}

// Calculate the SHA1 hash for a message.
// Based on pseudocode from Wikipedia: https://en.wikipedia.org/wiki/SHA-1#SHA-1_pseudocode
void sha1(const unsigned char* msg, const size_t msg_size, unsigned char (&hash)[20]) {
  // Make a copy of the message into a new buffer.
  std::vector<unsigned char> message;
  message.assign(msg, msg + msg_size);

  // Set the first bit after the message to 1.
  message.push_back(0x80u);

  // Pad the message to a multiple of 512 bits (i.e. 64 characters), minus the 64-bit size (see
  // below).
  size_t padding = 64u - (message.size() % 64u);
  padding = (padding >= 8) ? padding - 8 : padding + 64 - 8;
  message.insert(message.end(), padding, 0u);

  // Append the original length, in bits, as a 64-bit big endian number.
  const uint64_t original_size_bits = static_cast<uint64_t>(msg_size) * 8u;
  for (int i = 0; i < 8; ++i) {
    message.push_back(static_cast<unsigned char>(original_size_bits >> (56 - 8 * i)));
  }

  // Initial state of the hash.
  uint32_t h0 = 0x67452301u;
  uint32_t h1 = 0xEFCDAB89u;
  uint32_t h2 = 0x98BADCFEu;
  uint32_t h3 = 0x10325476u;
  uint32_t h4 = 0xC3D2E1F0u;

  // Work buffer for each chunk.
  uint32_t w[80];

  // Loop over all 512-bit chunks.
  const size_t num_chunks = message.size() / 64;
  for (size_t j = 0; j < num_chunks; ++j) {
    // Extract the chunk as sixteen 32-bit words.
    for (size_t i = 0; i < 16; ++i) {
      w[i] = get_uint32_be(&message[(j * 64) + (i * 4)]);
    }

    // Extend the sixteen 32-bit words into eighty 32-bit words.
    for (size_t i = 16; i < 80; ++i) {
      uint32_t temp = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16];
      temp = (temp << 1) + (temp >> 31);
      w[i] = temp;
    }

    // Initialize hash value for this chunk.
    uint32_t a = h0;
    uint32_t b = h1;
    uint32_t c = h2;
    uint32_t d = h3;
    uint32_t e = h4;

    // Main loop.
    for (size_t i = 0; i < 80; ++i) {
      uint32_t f;
      uint32_t k;
      if (i < 20) {
        f = (b & c) | ((~b) & d);
        k = 0x5A827999u;
      } else if (i < 40) {
        f = b ^ c ^ d;
        k = 0x6ED9EBA1u;
      } else if (i < 60) {
        f = (b & c) | (b & d) | (c & d);
        k = 0x8F1BBCDCu;
      } else {
        f = b ^ c ^ d;
        k = 0xCA62C1D6u;
      }

      f = ((a << 5) | (a >> 27)) + f + e + k + w[i];
      e = d;
      d = c;
      c = (b << 30) | (b >> 2);
      b = a;
      a = f;
    }

    // Add this chunk's hash to result so far.
    h0 = h0 + a;
    h1 = h1 + b;
    h2 = h2 + c;
    h3 = h3 + d;
    h4 = h4 + e;
  }

  // Write the hash to the output buffer.
  set_uint32_be(h0, &hash[0]);
  set_uint32_be(h1, &hash[4]);
  set_uint32_be(h2, &hash[8]);
  set_uint32_be(h3, &hash[12]);
  set_uint32_be(h4, &hash[16]);
}

void prepare_hmac_sha1_key(const char* key, unsigned char (&key_pad)[64]) {
  size_t key_len = std::strlen(key);

  if (key_len > 64) {
    // Keys longer than 64 characters are shortened by hashing them (it becomes 20 bytes long).
    unsigned char hash[20];
    sha1(reinterpret_cast<const unsigned char*>(key), key_len, hash);
    std::memcpy(&key_pad[0], &hash[0], 20);
    key_len = 20;
  } else {
    std::memcpy(&key_pad[0], key, key_len);
  }

  // Keys shorter than 64 characters are padded with zeros on the right.
  if (key_len < 64) {
    std::memset(&key_pad[key_len], 0, 64 - key_len);
  }
}

}  // namespace

// Based on pseudocode from Wikipedia: https://en.wikipedia.org/wiki/HMAC#Implementation
result_t<hmac_sha1_t> hmac_sha1(const char* key, const char* data) {
  // Prepare the key (make it exactly 64 characters long).
  unsigned char key_pad[64];
  prepare_hmac_sha1_key(key, key_pad);

  // Outer and innder keys, padded.
  unsigned char outer_key_pad[64];
  unsigned char inner_key_pad[64];
  for (int i = 0; i < 64; ++i) {
    outer_key_pad[i] = key_pad[i] ^ 0x5Cu;
    inner_key_pad[i] = key_pad[i] ^ 0x36u;
  }

  // Inner hash.
  unsigned char inner_hash[20];
  {
    // Concatenate inner_key_pad + data.
    const size_t data_len = std::strlen(data);
    std::vector<unsigned char> msg(sizeof(inner_key_pad) + data_len);
    std::memcpy(&msg[0], &inner_key_pad[0], sizeof(inner_key_pad));
    std::memcpy(&msg[sizeof(inner_key_pad)], &data[0], data_len);

    // Calculate the inner hash.
    sha1(&msg[0], msg.size(), inner_hash);
  }

  // Outer hash (i.e. the result)
  unsigned char outer_hash[20];
  {
    // Concatenate outer_key_pad + inner_hash.
    std::vector<unsigned char> msg(sizeof(outer_key_pad) + sizeof(inner_hash));
    std::memcpy(&msg[0], &outer_key_pad[0], sizeof(outer_key_pad));
    std::memcpy(&msg[sizeof(outer_key_pad)], &inner_hash[0], sizeof(inner_hash));

    // Calculate the outer hash.
    sha1(&msg[0], msg.size(), outer_hash);
  }

  return make_result(hmac_sha1_t(outer_hash));
}

}  // namespace us3
