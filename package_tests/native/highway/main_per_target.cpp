/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for highway: per target; calls hwy::VectorBytes(), hwy::HaveInteger64(), hwy::HaveFloat64(), hwy::HaveFloat16().
 */

#include <hwy/highway.h>
#include <hwy/per_target.h>
#include <hwy/targets.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

int main() {
    /* 1. VectorBytes() must be a positive power-of-two on any healthy target.
     * Even the scalar fallback reports >= sizeof(uint8_t) = 1. */
    const size_t vbytes = hwy::VectorBytes();
    assert(vbytes >= 1);
    /* Power-of-two check: (n & (n-1)) == 0 for n>0. */
    assert((vbytes & (vbytes - 1)) == 0);
    assert(vbytes <= 4096);

    const bool have_i64 = hwy::HaveInteger64();
    assert(have_i64);

    const bool have_f64 = hwy::HaveFloat64();
    const int have_f64_int = static_cast<int>(have_f64);
    assert(have_f64_int == 0 || have_f64_int == 1);

    const bool have_f16 = hwy::HaveFloat16();
    const int have_f16_int = static_cast<int>(have_f16);
    assert(have_f16_int == 0 || have_f16_int == 1);

    /* 5. DispatchedTarget() — the target the dynamic dispatcher selected.
     * Must be non-zero (0 means dispatch never ran / table empty) and must
     * resolve to a known TargetName (not "Unknown" — that would mean the
     * library returned a corrupt bit pattern). */
    const int64_t dispatched = hwy::DispatchedTarget();
    assert(dispatched != 0);
    const char *name = hwy::TargetName(dispatched);
    assert(name != nullptr);
    /* "Unknown" is the targets.h fallback for an unrecognized bit — would
     * indicate ABI breakage between the header and the .so. */
    assert(std::strcmp(name, "Unknown") != 0);

    std::printf("highway per-target test OK: vbytes=%zu i64=%d f64=%d f16=%d dispatched=%s\n",
                vbytes, (int)have_i64, (int)have_f64, (int)have_f16, name);
    return 0;
}
