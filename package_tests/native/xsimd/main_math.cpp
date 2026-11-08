/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xsimd: math; calls out(), batch_t::load_aligned().
 */

#include <xsimd/xsimd.hpp>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <vector>

int main() {
    using batch_t = xsimd::batch<float>;
    constexpr std::size_t N = batch_t::size;

    std::vector<float, xsimd::default_allocator<float>> in(N), out(N);

    /* 1. sqrt: in[i] = (i+1)^2 -> sqrt -> i+1. */
    for (std::size_t i = 0; i < N; ++i) {
        const float v = static_cast<float>(i + 1);
        in[i] = v * v;
    }
    const batch_t vin = batch_t::load_aligned(in.data());
    const batch_t vsqrt = xsimd::sqrt(vin);
    vsqrt.store_aligned(out.data());
    for (std::size_t i = 0; i < N; ++i) {
        const float expected = static_cast<float>(i + 1);
        assert(std::fabs(out[i] - expected) < 1e-4f);
    }

    /* 2. sin^2 + cos^2 == 1 (within tolerance, since these are
     *    polynomial approximations on most archs). */
    for (std::size_t i = 0; i < N; ++i) {
        in[i] = 0.1f * static_cast<float>(i);
    }
    const batch_t vx = batch_t::load_aligned(in.data());
    const batch_t vsin = xsimd::sin(vx);
    const batch_t vcos = xsimd::cos(vx);
    const batch_t vone = vsin * vsin + vcos * vcos;
    vone.store_aligned(out.data());
    for (std::size_t i = 0; i < N; ++i) {
        assert(std::fabs(out[i] - 1.0f) < 1e-4f);
    }

    /* 3. exp: exp(0) == 1, exp(log(2)) ~= 2. */
    for (std::size_t i = 0; i < N; ++i) {
        in[i] = (i == 1) ? std::log(2.0f) : 0.0f;
    }
    const batch_t vlog = batch_t::load_aligned(in.data());
    const batch_t vexp = xsimd::exp(vlog);
    vexp.store_aligned(out.data());
    assert(std::fabs(out[0] - 1.0f) < 1e-5f);
    if (N >= 2) {
        assert(std::fabs(out[1] - 2.0f) < 1e-4f);
    }

    return 0;
}
