/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for xsimd: bool select; calls out(), batch_t::load_aligned(), xsimd::any(), xsimd::all().
 */

#include <xsimd/xsimd.hpp>
#include <cassert>
#include <cstddef>
#include <vector>

int main() {
    using batch_t = xsimd::batch<float>;
    constexpr std::size_t N = batch_t::size;

    std::vector<float, xsimd::default_allocator<float>> a(N), b(N), out(N);
    /* a = [0, 1, 2, ...], b = constant 1.5 -> a < b is true only for
     * lanes 0 and 1 (when 0 < 1.5 and 1 < 1.5). */
    for (std::size_t i = 0; i < N; ++i) {
        a[i] = static_cast<float>(i);
        b[i] = 1.5f;
    }

    const batch_t va = batch_t::load_aligned(a.data());
    const batch_t vb = batch_t::load_aligned(b.data());

    /* 1. Comparison produces batch_bool. */
    const auto mask_lt = va < vb;

    /* 2. any() / all() reductions. */
    assert(xsimd::any(mask_lt));        /* lanes 0 and 1 are true */
    assert(!xsimd::all(mask_lt));

    /* 3. select(): pick a where the mask is set, else b. */
    const batch_t vsel = xsimd::select(mask_lt, va, vb);
    vsel.store_aligned(out.data());
    for (std::size_t i = 0; i < N; ++i) {
        const float expected = (a[i] < b[i]) ? a[i] : b[i];
        assert(out[i] == expected);
    }

    /* Sanity: an all-false mask returns the else-branch entirely. */
    const auto mask_gt = va > batch_t(1e30f);
    assert(!xsimd::any(mask_gt));
    const batch_t vall_else = xsimd::select(mask_gt, va, vb);
    vall_else.store_aligned(out.data());
    for (std::size_t i = 0; i < N; ++i) {
        assert(out[i] == b[i]);
    }

    return 0;
}
