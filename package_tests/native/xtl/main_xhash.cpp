/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for xtl: xhash; calls xtl::hash_bytes(), xtl::murmur2_x86(), xtl::murmur2_x64().
 */

#include <xtl/xhash.hpp>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

int main() {
    const std::size_t kSeed = 0;

    /* Deterministic: hashing the same bytes twice yields identical output. */
    const std::string s1 = "demo-xtl-hash-probe";
    const auto h1a = xtl::hash_bytes(s1.data(), s1.size(), kSeed);
    const auto h1b = xtl::hash_bytes(s1.data(), s1.size(), kSeed);
    assert(h1a == h1b);

    const std::string s2 = "demo-xtl-hash-probf"; /* one-char diff */
    assert(s1.size() == s2.size());
    const auto h2 = xtl::hash_bytes(s2.data(), s2.size(), kSeed);
    assert(h1a != h2);

    /* Explicit-width primitives: both must be deterministic and must
       distinguish the two distinct buffers. Relational checks only — no
       hard-coded hash constant. */
    const std::uint32_t a32 = xtl::murmur2_x86(s1.data(), s1.size(),
                                               static_cast<std::uint32_t>(kSeed));
    const std::uint32_t b32 = xtl::murmur2_x86(s1.data(), s1.size(),
                                               static_cast<std::uint32_t>(kSeed));
    const std::uint32_t c32 = xtl::murmur2_x86(s2.data(), s2.size(),
                                               static_cast<std::uint32_t>(kSeed));
    assert(a32 == b32);
    assert(a32 != c32);

    const std::uint64_t a64 = xtl::murmur2_x64(s1.data(), s1.size(),
                                               static_cast<std::uint64_t>(kSeed));
    const std::uint64_t b64 = xtl::murmur2_x64(s1.data(), s1.size(),
                                               static_cast<std::uint64_t>(kSeed));
    const std::uint64_t c64 = xtl::murmur2_x64(s2.data(), s2.size(),
                                               static_cast<std::uint64_t>(kSeed));
    assert(a64 == b64);
    assert(a64 != c64);

    std::cout << "xtl xhash test OK" << std::endl;
    return 0;
}
