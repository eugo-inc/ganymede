/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xsimd: fma; calls out(), batch_t::load_aligned(), xsimd::fma(), xsimd::fms().
 */

#include <xsimd/xsimd.hpp>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <vector>

int main() {
    using batch_t = xsimd::batch<float>;
    constexpr std::size_t N = batch_t::size;

    std::vector<float, xsimd::default_allocator<float>> a(N), b(N), c(N), out(N);
    for (std::size_t i = 0; i < N; ++i) {
        a[i] = static_cast<float>(i + 1);         /* 1, 2, 3, ... */
        b[i] = static_cast<float>(2 * i + 3);     /* 3, 5, 7, ... */
        c[i] = static_cast<float>(10 + i);        /* 10, 11, 12, ... */
    }

    const batch_t va = batch_t::load_aligned(a.data());
    const batch_t vb = batch_t::load_aligned(b.data());
    const batch_t vc = batch_t::load_aligned(c.data());

    /* 1. fma: a * b + c */
    const batch_t vfma = xsimd::fma(va, vb, vc);
    vfma.store_aligned(out.data());
    for (std::size_t i = 0; i < N; ++i) {
        const float expected = a[i] * b[i] + c[i];
        assert(std::fabs(out[i] - expected) < 1e-3f);
    }

    /* 2. fms: a * b - c */
    const batch_t vfms = xsimd::fms(va, vb, vc);
    vfms.store_aligned(out.data());
    for (std::size_t i = 0; i < N; ++i) {
        const float expected = a[i] * b[i] - c[i];
        assert(std::fabs(out[i] - expected) < 1e-3f);
    }

    /* 3. fnma: -(a * b) + c */
    const batch_t vfnma = xsimd::fnma(va, vb, vc);
    vfnma.store_aligned(out.data());
    for (std::size_t i = 0; i < N; ++i) {
        const float expected = -(a[i] * b[i]) + c[i];
        assert(std::fabs(out[i] - expected) < 1e-3f);
    }

    /* 4. fnms: -(a * b) - c */
    const batch_t vfnms = xsimd::fnms(va, vb, vc);
    vfnms.store_aligned(out.data());
    for (std::size_t i = 0; i < N; ++i) {
        const float expected = -(a[i] * b[i]) - c[i];
        assert(std::fabs(out[i] - expected) < 1e-3f);
    }

    return 0;
}
