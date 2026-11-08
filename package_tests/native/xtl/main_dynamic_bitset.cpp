/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xtl: dynamic bitset; compiles against <xtl/xdynamic_bitset.hpp>.
 */

#include <xtl/xdynamic_bitset.hpp>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>

int main() {
    /* Size > 64 bits forces at least 2 storage blocks on a 64-bit word
       backing — exercises the multi-word stride path. */
    constexpr std::size_t N = 200;
    xtl::xdynamic_bitset<std::uint64_t> bs(N);

    assert(bs.size() == N);
    /* Newly-constructed bitset is documented to be zero-filled. */
    assert(bs.count() == 0);

    bs[0] = true;
    bs[63] = true;       /* last bit of block 0 */
    bs[64] = true;       /* first bit of block 1 */
    bs[127] = true;      /* last bit of block 1 */
    bs[128] = true;      /* first bit of block 2 */
    bs[N - 1] = true;    /* last addressable bit */

    assert(bool(bs[0]) == true);
    assert(bool(bs[63]) == true);
    assert(bool(bs[64]) == true);
    assert(bool(bs[127]) == true);
    assert(bool(bs[128]) == true);
    assert(bool(bs[N - 1]) == true);
    assert(bool(bs[1]) == false);
    assert(bool(bs[100]) == false);

    /* popcount across all blocks. */
    assert(bs.count() == 6);

    /* Clear one and re-check. */
    bs[64] = false;
    assert(bool(bs[64]) == false);
    assert(bs.count() == 5);

    std::cout << "xtl xdynamic_bitset test OK" << std::endl;
    return 0;
}
