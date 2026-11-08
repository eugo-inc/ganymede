/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xsimd: core API; calls out(), batch_t::load_aligned(), xsimd::reduce_add(), batch_t::arch_type::name().
 */

#include <xsimd/xsimd.hpp>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

int main() {
    using batch_t = xsimd::batch<float>;
    constexpr std::size_t N = batch_t::size;

    /* Two aligned lanes worth of inputs + a destination. */
    std::vector<float, xsimd::default_allocator<float>> a(N), b(N), out(N);
    for (std::size_t i = 0; i < N; ++i) {
        a[i] = static_cast<float>(i);
        b[i] = static_cast<float>(10 * i + 1);
    }

    /* Load -> add -> store. This is the canonical xsimd op path; if any of
     * the three primitives is broken at the codegen level, this fails to
     * link OR aborts at runtime. */
    const batch_t va = batch_t::load_aligned(a.data());
    const batch_t vb = batch_t::load_aligned(b.data());
    const batch_t vsum = va + vb;
    vsum.store_aligned(out.data());

    for (std::size_t i = 0; i < N; ++i) {
        assert(out[i] == a[i] + b[i]);
    }

    /* Reduce-add as a second op family — exercises the horizontal-reduction
     * codegen path, which is arch-specific (different from elementwise add). */
    const float total = xsimd::reduce_add(vsum);
    float expected_total = 0.0f;
    for (std::size_t i = 0; i < N; ++i) {
        expected_total += a[i] + b[i];
    }
    assert(total == expected_total);

    std::cout << "xsimd test OK: batch<float>::size=" << N
              << " arch=" << batch_t::arch_type::name() << std::endl;
    return 0;
}
