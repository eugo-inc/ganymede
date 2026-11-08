/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for google_crc32c: cpp api; calls crc32c::Crc32c(), crc32c::Extend().
 */

#include <crc32c/crc32c.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <string>

int main() {
    static const std::uint8_t input[] = {
        '1', '2', '3', '4', '5', '6', '7', '8', '9',
    };
    const std::uint32_t expected = 0xe3069283u;

    /* uint8_t* overload (the primitive). */
    std::uint32_t via_bytes = crc32c::Crc32c(input, sizeof(input));
    assert(via_bytes == expected);

    /* char* overload (reinterpret_cast shim in the header). */
    const char* as_chars = "123456789";
    std::uint32_t via_chars = crc32c::Crc32c(as_chars, 9);
    assert(via_chars == expected);

    /* std::string overload (forwards to char* overload). */
    std::string as_string = "123456789";
    std::uint32_t via_string = crc32c::Crc32c(as_string);
    assert(via_string == expected);

    /* Extend() with non-zero seed must match the two-chunk split. */
    std::uint32_t seed = crc32c::Extend(0, input, 4);
    std::uint32_t composed = crc32c::Extend(seed, input + 4, sizeof(input) - 4);
    assert(composed == expected);

    std::printf("google_crc32c C++ API OK: crc32c::Crc32c(\"123456789\") = 0x%08x\n",
                via_string);
    return 0;
}
