/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xsimd: unaligned; calls batch_t::load_unaligned().
 */

#include <xsimd/xsimd.hpp>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

int main() {
    using batch_t = xsimd::batch<float>;
    constexpr std::size_t N = batch_t::size;

    std::vector<float> raw(N + 1);
    for (std::size_t i = 0; i < N + 1; ++i) {
        raw[i] = static_cast<float>(i * 3 + 1);
    }
    float* unaligned_ptr = raw.data() + 1;

    /* 1. Unaligned load + unaligned store round-trip. */
    const batch_t vunalign = batch_t::load_unaligned(unaligned_ptr);
    std::vector<float> out_unaligned(N + 1, 0.0f);
    vunalign.store_unaligned(out_unaligned.data() + 1);
    for (std::size_t i = 0; i < N; ++i) {
        assert(out_unaligned[i + 1] == raw[i + 1]);
    }

    /* 2. Broadcast-from-scalar constructor: every lane equals the scalar. */
    const float scalar = 3.14159f;
    const batch_t vbcast(scalar);
    std::vector<float, xsimd::default_allocator<float>> bcast_out(N);
    vbcast.store_aligned(bcast_out.data());
    for (std::size_t i = 0; i < N; ++i) {
        assert(std::fabs(bcast_out[i] - scalar) < 1e-6f);
    }

    /* 3. get(i) per-lane accessor. */
    for (std::size_t i = 0; i < N; ++i) {
        const float lane = vunalign.get(i);
        assert(lane == raw[i + 1]);
    }

    /* 4. Mixed-alignment path: unaligned load -> arithmetic -> aligned store.
     *    Add a broadcast scalar to the unaligned-loaded batch, then store
     *    to aligned memory. Cross-checks the actual codegen does the right
     *    thing when source + destination have different alignment requirements. */
    const batch_t vsum = vunalign + vbcast;
    std::vector<float, xsimd::default_allocator<float>> aligned_out(N);
    vsum.store_aligned(aligned_out.data());
    for (std::size_t i = 0; i < N; ++i) {
        const float expected = raw[i + 1] + scalar;
        assert(std::fabs(aligned_out[i] - expected) < 1e-4f);
    }

    return 0;
}
