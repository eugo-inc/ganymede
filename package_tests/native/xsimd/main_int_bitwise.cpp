/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xsimd: int bitwise; calls out(), batch_t::load_aligned().
 */

#include <xsimd/xsimd.hpp>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

int main() {
    using batch_t = xsimd::batch<int32_t>;
    constexpr std::size_t N = batch_t::size;

    std::vector<int32_t, xsimd::default_allocator<int32_t>> a(N), b(N), out(N);
    for (std::size_t i = 0; i < N; ++i) {
        a[i] = static_cast<int32_t>(0xA5A5A5A5u ^ (i * 7u));
        b[i] = static_cast<int32_t>(0x5A5A5A5Au + (i * 13u));
    }

    const batch_t va = batch_t::load_aligned(a.data());
    const batch_t vb = batch_t::load_aligned(b.data());

    /* 1. Integer add. */
    const batch_t vadd = va + vb;
    vadd.store_aligned(out.data());
    for (std::size_t i = 0; i < N; ++i) {
        assert(out[i] == static_cast<int32_t>(a[i] + b[i]));
    }

    /* 2. Bitwise AND. */
    const batch_t vand = va & vb;
    vand.store_aligned(out.data());
    for (std::size_t i = 0; i < N; ++i) {
        assert(out[i] == (a[i] & b[i]));
    }

    /* 3. Bitwise OR. */
    const batch_t vor = va | vb;
    vor.store_aligned(out.data());
    for (std::size_t i = 0; i < N; ++i) {
        assert(out[i] == (a[i] | b[i]));
    }

    /* 4. Bitwise XOR — also serves as a self-inverse check: a XOR a == 0. */
    const batch_t vxor = va ^ vb;
    vxor.store_aligned(out.data());
    for (std::size_t i = 0; i < N; ++i) {
        assert(out[i] == (a[i] ^ b[i]));
    }

    const batch_t vxor_self = va ^ va;
    vxor_self.store_aligned(out.data());
    for (std::size_t i = 0; i < N; ++i) {
        assert(out[i] == 0);
    }

    return 0;
}
