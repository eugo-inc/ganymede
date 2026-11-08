/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xsimd: complex; calls out(), batch_t::load_aligned().
 */

#include <xsimd/xsimd.hpp>
#include <cassert>
#include <cmath>
#include <complex>
#include <cstddef>
#include <vector>

int main() {
    using cf = std::complex<float>;
    using batch_t = xsimd::batch<cf>;
    using real_batch_t = xsimd::batch<float>;
    constexpr std::size_t N = batch_t::size;

    /* In xsimd a complex batch stores its real and imaginary parts in two
     * separate real registers, so it holds the SAME number of (complex) lanes
     * as the underlying real batch holds float lanes — NOT half. The header
     * defines `batch<std::complex<T>>::size = real_batch::size`. */
    static_assert(batch_t::size == real_batch_t::size,
                  "complex batch lane count must equal the real batch lane count");

    std::vector<cf, xsimd::default_allocator<cf>> a(N), b(N), out(N);
    for (std::size_t i = 0; i < N; ++i) {
        a[i] = cf(static_cast<float>(i + 1), static_cast<float>(2 * i + 1));
        b[i] = cf(static_cast<float>(3 * i + 2), static_cast<float>(i + 7));
    }

    const batch_t va = batch_t::load_aligned(a.data());
    const batch_t vb = batch_t::load_aligned(b.data());

    /* 1. Complex addition: component-wise on real and imag parts. */
    const batch_t vadd = va + vb;
    vadd.store_aligned(out.data());
    for (std::size_t i = 0; i < N; ++i) {
        const cf expected = a[i] + b[i];
        assert(std::fabs(out[i].real() - expected.real()) < 1e-4f);
        assert(std::fabs(out[i].imag() - expected.imag()) < 1e-4f);
    }

    /* 2. Complex multiplication: the non-trivial path (shuffles + fma). */
    const batch_t vmul = va * vb;
    vmul.store_aligned(out.data());
    for (std::size_t i = 0; i < N; ++i) {
        const cf expected = a[i] * b[i];
        assert(std::fabs(out[i].real() - expected.real()) < 1e-3f);
        assert(std::fabs(out[i].imag() - expected.imag()) < 1e-3f);
    }

    return 0;
}
